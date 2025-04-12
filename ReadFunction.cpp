#include "ReadFunction.h"

Header ReadFunction::ReadBinaryHeader(const string& fileName){
    try{
        // Parsea el nombre
        string nameBinary = fileName.substr(0, fileName.find_last_of("."));
        // Abre el archivo binario
        ifstream fileHeader2(nameBinary + ".bin", std::ios::binary);
        if (!fileHeader2.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        // Crea el header
        Header headerJson2;
        // Lee el header
        int sizeFields;
        int sizeSecondary;
        // Obtiene el espacio de los vectores
        fileHeader2.read(reinterpret_cast<char*>(&sizeFields), sizeof(int));
        fileHeader2.read(reinterpret_cast<char*>(&sizeSecondary), sizeof(int));
        // Lee los vectores
        for (int i = 0; i < sizeFields; i++){
            int sizeCampo;
            fileHeader2.read(reinterpret_cast<char*>(&sizeCampo), sizeof(int));
            char campo [sizeCampo];
            int sizeType;
            int valorLength;
            fileHeader2.read(campo, sizeCampo);
            string campoString(campo);
            campoString = campoString.substr(0, sizeCampo);
            headerJson2.fieldsName.push_back(campoString);
            fileHeader2.read(reinterpret_cast<char*>(&sizeType), sizeof(int));
            char type [sizeType];
            fileHeader2.read(type, sizeType);
            string typeString(type);
            typeString = typeString.substr(0, sizeType);
            headerJson2.fieldsType.push_back(typeString);
            fileHeader2.read(reinterpret_cast<char*>(&valorLength), sizeof(int));
            headerJson2.fieldsSize.push_back(valorLength);
        }
        for (int i = 0; i < sizeSecondary; i++){
            int sizeSecondary;
            fileHeader2.read(reinterpret_cast<char*>(&sizeSecondary), sizeof(int));
            char secondary [sizeSecondary];
            fileHeader2.read(secondary, sizeSecondary);
            string secondaryString(secondary);
            secondaryString = secondaryString.substr(0, sizeSecondary);
            headerJson2.secondaryKey.push_back(secondaryString);
        }
        // Lee la llave primaria
        int sizePrimaryKey;
        fileHeader2.read(reinterpret_cast<char*>(&sizePrimaryKey), sizeof(int));
        char primaryKey [sizePrimaryKey];
        fileHeader2.read(primaryKey, sizePrimaryKey);
        string primaryKeyString(primaryKey);
        primaryKeyString = primaryKeyString.substr(0, sizePrimaryKey);
        headerJson2.primaryKey = primaryKeyString;
        // Lee el availist
        fileHeader2.read(reinterpret_cast<char*>(&headerJson2.refAvailList), sizeof(int));
        // Lee la cantidad de registros
        fileHeader2.read(reinterpret_cast<char*>(&headerJson2.cantRegistros), sizeof(int));
        fileHeader2.close();

        return headerJson2;

    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
        Header header;
        return header;
    }
}

void ReadFunction::CargarDatos(const string& fileName, const string& fileName2){
    try{
        // Vectores necesarios para guardar los datos y validar si deben escribirse o no
        vector<string> registros;
        vector<int> sizeRegistro;
        int contNewRecords = 0;
        // Vector para verificar hay llave primaria duplicada
        vector<string> llavesPrimarias;
        int sizePrimaryKey = 0;
        string llavePrimaria;
        // Llave secundaria
        vector<string> refLlavePrimaria;
        vector<int> sizeLlaveSecundaria;
        vector<int> offsetLlaveSecundaria;
        // leer el archivo binario con el header y abrir el archivo csv
        string nameBinary = fileName.substr(0, fileName.find_last_of("."));
        string nameCsv = fileName2.substr(0, fileName2.find_last_of("."));
        // lee el header
        Header header = ReadBinaryHeader(fileName);
        if (header.cantRegistros > 0){
            int sizeFields = 0;
            for (int i = 0; i < static_cast<int>(header.fieldsSize.size()); i++){
                sizeFields += header.fieldsSize [i];
            }
            vector<string> extra = ReadBinaryRecordsPrimaryKey(header, sizeFields, fileName);
            llavesPrimarias.resize(extra.size());
            copy(extra.begin(), extra.end(), llavesPrimarias.begin());
        }
        if (header.fieldsName.empty()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        ifstream fileCsv(nameCsv + ".csv");
        if (!fileCsv){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"csv file not found\" \n");
        }
        if (!fileCsv.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"csv file not found\" \n");
        }
        // lee el archivo csv y sus campos
        string line;
        int cantRegistros = 0;
        int cantSkipped = 0;
        fileCsv.seekg(0, ios::beg);
        while (getline(fileCsv, line)){
            // Leer los campos
            stringstream ss(line);
            string field;
            int i = 0;
            registros.clear();
            sizeRegistro.clear();
            bool isValid = true;
            while (getline(ss, field, ',')){
                // Significa que esta en el header del csv
                if (cantRegistros == 0){
                    // Valida si el numero de campos es igual al numero de campos del header
                    if (i >= static_cast<int>(header.fieldsName.size())){
                        throw runtime_error(
                                "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"CSV fields do not match file structure\"} \n");
                    }
                    // Valida si el tipo de campo es igual al tipo de campo del header
                    if (field != header.fieldsName [i]){
                        throw runtime_error(
                                "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"CSV fields do not match file structure\"} \n");
                    }
                } else{
                    // Valida si el campo es una llave primaria
                    bool isSecondary = false;
                    if (field == "" && header.primaryKey == header.fieldsName [i]){
                        cantSkipped++;
                        isValid = false;
                        // Salta el campo
                        for (i = i; i < static_cast<int>(header.fieldsName.size()); i++){
                            getline(ss, field, ',');
                        }
                        // Si no esta la llave secundaria
                    } else if (field == ""){
                        for (int j = 0; j < static_cast<int>(header.secondaryKey.size()); j++){
                            if (header.secondaryKey [j] == header.fieldsName [i]){
                                isSecondary = true;
                                isValid = false;
                                cantSkipped++;
                                // Salta el campo
                                for (i = i; i < static_cast<int>(header.fieldsName.size()); i++){
                                    getline(ss, field, ',');
                                }
                                break;
                            }
                        }
                        // Si el campo es tipo char
                    }
                    if (isSecondary == false){
                        if (header.fieldsType [i] == "char"){
                            // Validar si el caracter es mayor al tamaño del campo
                            if (field.length() > static_cast<size_t>(header.fieldsSize [i])){
                                field = field.substr(0, header.fieldsSize [i]);
                            }
                            // Agrega el campo que es de char
                            registros.push_back(field);
                            sizeRegistro.push_back(header.fieldsSize [i]);
                            // Si el campo es tipo entero
                            // Validar si es primary key para hacer el index
                            if (header.fieldsName [i] == header.primaryKey){
                                // si esta vacio solo agregue la llave
                                llavePrimaria = field;
                                if (llavesPrimarias.empty()){
                                    sizePrimaryKey = header.fieldsSize [i];
                                    llavesPrimarias.push_back(llavePrimaria);
                                    // Sino verifique primero que no este repetida
                                } else{
                                    if (UniqueIDMethod(llavesPrimarias, llavePrimaria)){
                                        llavesPrimarias.push_back(llavePrimaria);
                                    } else{
                                        cantSkipped++;
                                        isValid = false;
                                        // Salta el campo
                                        for (i = i; i < static_cast<int>(header.fieldsName.size()); i++){
                                            getline(ss, field, ',');
                                        }
                                    }
                                }

                            } else{
                                // Valida si es una llave secundaria para hacer el index
                                for (int j = 0; j < static_cast<int>(header.secondaryKey.size()); j++){
                                    if (header.secondaryKey [j] == header.fieldsName [i]){
                                        refLlavePrimaria.push_back(llavePrimaria);
                                        break;
                                    }
                                }
                            }
                        } else if (header.fieldsType [i] == "int"){
                            regex Integer("^[0-9]+$");
                            if (!regex_match(field, Integer) && header.primaryKey == header.fieldsName [i]){
                                // Saltarse ese registro
                                cantSkipped++;
                                isValid = false;
                                for (i = i; i < static_cast<int>(header.fieldsName.size()); i++){
                                    getline(ss, field, ',');
                                }
                            } else if (!regex_match(field, Integer)){
                                for (int j = 0; j < static_cast<int>(header.secondaryKey.size()); j++){
                                    if (header.secondaryKey [j] == header.fieldsName [i]){
                                        isValid = false;
                                        isSecondary = true;
                                        cantSkipped++;
                                        // Salta el campo
                                        for (i = i; i < static_cast<int>(header.fieldsName.size()); i++){
                                            getline(ss, field, ',');
                                        }
                                        break;
                                    }
                                }
                            }
                            if (isSecondary == false){
                                if (!regex_match(field, Integer) || field == ""){
                                    registros.push_back("0");
                                    sizeRegistro.push_back(header.fieldsSize [i]);
                                } else{
                                    // Agrega el campo que es de int
                                    registros.push_back(field);
                                    sizeRegistro.push_back(header.fieldsSize [i]);
                                }
                            }
                            // Validar si es primary key para hacer el index
                            if (header.fieldsName [i] == header.primaryKey){
                                // si esta vacio solo agregue la llave
                                llavePrimaria = field;
                                if (llavesPrimarias.empty()){
                                    sizePrimaryKey = header.fieldsSize [i];
                                    llavesPrimarias.push_back(llavePrimaria);
                                    // Sino verifique primero que no este repetida
                                } else{
                                    if (UniqueIDMethod(llavesPrimarias, llavePrimaria)){
                                        llavesPrimarias.push_back(llavePrimaria);
                                    } else{
                                        cantSkipped++;
                                        isValid = false;
                                        // Salta el campo
                                        for (i = i; i < static_cast<int>(header.fieldsName.size()); i++){
                                            getline(ss, field, ',');
                                        }
                                    }
                                }

                            } else{
                                // Valida si es una llave secundaria para hacer el index
                                for (int j = 0; j < static_cast<int>(header.secondaryKey.size()); j++){
                                    if (header.secondaryKey [j] == header.fieldsName [i]){
                                        refLlavePrimaria.push_back(llavePrimaria);
                                        break;
                                    }
                                }
                            }
                            // Si el campo es tipo float
                        } else if (header.fieldsType [i] == "float"){
                            regex Float("^[0-9]+[.][0-9]+$");
                            if (!regex_match(field, Float) && header.primaryKey == header.fieldsName [i]){
                                // Saltarse ese registro
                                cantSkipped++;
                                for (i = i; i < static_cast<int>(header.fieldsName.size()); i++){
                                    getline(ss, field, ',');
                                }
                            } else if (!regex_match(field, Float)){
                                for (int j = 0; j < static_cast<int>(header.secondaryKey.size()); j++){
                                    if (header.secondaryKey [j] == header.fieldsName [i]){
                                        isSecondary = true;
                                        cantSkipped++;
                                        // Salta el campo
                                        for (i = i; i < static_cast<int>(header.fieldsName.size()); i++){
                                            getline(ss, field, ',');
                                        }
                                        break;
                                    }
                                }
                            }
                            if (isSecondary == false){
                                if (!regex_match(field, Float)){
                                    registros.push_back("0");
                                    sizeRegistro.push_back(header.fieldsSize [i]);
                                } else{
                                    // Agrega el campo que es de float
                                    registros.push_back(field);
                                    sizeRegistro.push_back(header.fieldsSize [i]);
                                }
                            }
                            // Validar si es primary key para hacer el index
                            if (header.fieldsName [i] == header.primaryKey){
                                // si esta vacio solo agregue la llave
                                llavePrimaria = field;
                                if (llavesPrimarias.empty()){
                                    sizePrimaryKey = header.fieldsSize [i];
                                    llavesPrimarias.push_back(llavePrimaria);
                                    // Sino verifique primero que no este repetida
                                } else{
                                    if (UniqueIDMethod(llavesPrimarias, llavePrimaria)){
                                        llavesPrimarias.push_back(llavePrimaria);
                                    } else{
                                        cantSkipped++;
                                        isValid = false;
                                        // Salta el campo
                                        for (i = i; i < static_cast<int>(header.fieldsName.size()); i++){
                                            getline(ss, field, ',');
                                        }
                                    }
                                }

                            } else{
                                // Valida si es una llave secundaria para hacer el index
                                for (int j = 0; j < static_cast<int>(header.secondaryKey.size()); j++){
                                    if (header.secondaryKey [j] == header.fieldsName [i]){
                                        refLlavePrimaria.push_back(llavePrimaria);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                i++;
            }  // fin while campos
            if (isValid && cantRegistros > 0){
                // Agrega el registro al archivo binario
                if (header.refAvailList != -1){
                    string data;
                    for (int i = 0; i < static_cast<int>(registros.size()); i++){
                        data += header.fieldsName [i] + ":";
                        data += registros [i] + ",";
                    }
                    data = data.substr(0, data.size() - 1);
                    WriteFunction::AddRecord(fileName, data);
                    header = ReadBinaryHeader(fileName);
                } else{
                    WriteFunction::WriteBinaryHeaderRecords(fileName, registros, sizeRegistro, llavePrimaria, sizePrimaryKey, header);
                }

                contNewRecords++;
            }
            cantRegistros++;
        }
        fileCsv.close();
        if (cantSkipped > 0 && contNewRecords > 0){
        } else{
            cantRegistros = cantRegistros - 1;
            header.cantRegistros = cantRegistros;
        }
        if (cantSkipped > 0){
            cout << "{\"result\": \x1b[38;5;13m\"WARNING\"\x1b[0m, \"records\": "
                 << contNewRecords << " \"skipped\": " << cantSkipped << "}" << endl;
        } else{
            WriteFunction::WriteBinaryHeader(fileName, header);
            cout << "{\"result\": \x1b[38;5;10m\"OK\"\x1b[0m, \"records\": \""
                 << cantRegistros << "}" << endl;
        }
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

bool ReadFunction::UniqueIDMethod(vector<string>& primaryKeys, const string& primaryKey){
    unordered_set<string> set;
    for (const string& key : primaryKeys){
        if (set.count(key) > 0){
            return false;
        }
        set.insert(key);
    }
    return (set.count(primaryKey) == 0);
}

vector<string> ReadFunction::ReadBinaryRecordsPrimaryKey(const Header& header, int& sizeRecord, const string& fileName){
    vector<string> registros;
    // BORRAR
    vector<string> viewRegister;
    try{
        ifstream file(fileName, ios::binary | ios::in);
        if (!file.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        // Encontrar caracter '\n' en el archivo binario, con esto sabemos que el header termino ahi
        char caracFile;
        while (file.read(&caracFile, sizeof(char))){
            if (caracFile == '~'){
                break;
            }
        }

        for (int i = 0; i < static_cast<int>(header.cantRegistros); i++){
            char buffer [sizeRecord];
            file.read(buffer, sizeRecord);
            string record(buffer);
            stringstream ss(record);
            string field;
            int j = 0;
            viewRegister.push_back(record);
            while (getline(ss, field, ',')){
                if (field.substr(0, 1) == "*"){
                    i--;
                    break;
                }
                if (static_cast<string>(header.fieldsName [j]) == header.primaryKey){
                    registros.push_back(field);
                    break;
                }
                j++;
            }
        }
        return registros;
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
        return registros;
    }
}

void ReadFunction::DescribeBinary(const string& fileName){
    try{
        Header header = ReadBinaryHeader(fileName);
        if (header.fieldsName.empty()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        // Escribe en forma json
        cout << "\x1b[38;5;226m{\x1b[0m" << endl;
        cout << setw(2) << "";
        cout << "\x1b[38;5;117m\"fields\": \x1b[38;5;13m[\x1b[0m" << endl;
        for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
            if (i == static_cast<int>(header.fieldsName.size()) - 1){
                cout << setw(4) << ""
                     << "\x1b[38;5;4m{"
                     << "\x1b[38;5;117m\"name\": \x1b[0m"
                     << "\x1b[38;5;214m\"" << header.fieldsName [i] << "\"\x1b[0m, \x1b[38;5;117m\"type\": "
                     << "\x1b[38;5;214m\"" << header.fieldsType [i] << "\"\x1b[0m, \x1b[38;5;117m\"length\": "
                     << "\x1b[38;5;214m" << header.fieldsSize [i] << "\x1b[38;5;4m}\x1b[0m" << endl;
            } else{
                cout << setw(4) << ""
                     << "\x1b[38;5;4m{"
                     << "\x1b[38;5;117m\"name\": \x1b[0m"
                     << "\x1b[38;5;214m\"" << header.fieldsName [i] << "\"\x1b[0m, \x1b[38;5;117m\"type\": "
                     << "\x1b[38;5;214m\"" << header.fieldsType [i] << "\"\x1b[0m, \x1b[38;5;117m\"length\": "
                     << "\x1b[38;5;214m" << header.fieldsSize [i] << "\x1b[38;5;4m}\x1b[0m," << endl;
            }
        }
        cout << setw(2) << ""
             << "\x1b[38;5;13m]\x1b[0m," << endl;
        cout << setw(2) << ""
             << "\x1b[38;5;117m\"primary-key\": \x1b[0m"
             << "\x1b[38;5;214m\"" << header.primaryKey << "\"\x1b[0m," << endl;
        cout << setw(2) << ""
             << "\x1b[38;5;117m\"secondary-key\": \x1b[38;5;13m[\x1b[0m";
        for (int i = 0; i < static_cast<int>(header.secondaryKey.size()); i++){
            if (i == static_cast<int>(header.secondaryKey.size()) - 1){
                cout << "\x1b[38;5;214m\"" << header.secondaryKey [i] << "\"\x1b[0m";
            } else{
                cout << "\x1b[38;5;214m\"" << header.secondaryKey [i] << "\"\x1b[0m,";
            }
        }
        cout << "\x1b[38;5;13m]\x1b[0m," << endl;
        cout << setw(2) << ""
             << "\x1b[38;5;117m\"records\": \x1b[0m"
             << "\x1b[38;5;214m" << header.cantRegistros << "\x1b[0m," << endl;
        cout << "\x1b[38;5;226m}\x1b[0m" << endl;
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void ReadFunction::ListBinary(const string& fileName){
    Header header = ReadBinaryHeader(fileName);
    if (header.fieldsName.empty()){
        throw runtime_error(
                "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
    }
    ifstream file(fileName, ios::in | ios::binary);
    if (!file.is_open()){
        throw runtime_error(
                "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
    }
    int sizeRecord = 0;
    for (int i = 0; i < static_cast<int>(header.fieldsSize.size()); i++){
        sizeRecord += header.fieldsSize [i];
    }
    if (!file.is_open()){
        throw runtime_error(
                "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
    }
    // Encontrar caracter '\n' en el archivo binario, con esto sabemos que el header termino ahi
    char caracFile;
    while (file.read(&caracFile, sizeof(char))){
        if (caracFile == '~'){
            break;
        }
    }

    cout << "\x1b[38;5;201m[\x1b[0m" << endl;
    for (int i = 0; i < static_cast<int>(header.cantRegistros); i++){
        char buffer [sizeRecord];
        file.read(buffer, sizeRecord);
        string record(buffer);
        if (record [0] == '*'){
            i--;
        } else{
            cout << setw(1) << ""
                 << "\x1b[38;5;21m{\x1b[0m" << endl;
            stringstream ss(record);
            string field;
            int j = 0;
            while (getline(ss, field, ',')){
                if (j == static_cast<int>(header.fieldsName.size()) - 1){
                    if (header.fieldsType [j] == "int" || header.fieldsType [j] == "float"){
                        cout << setw(3) << ""
                             << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                        cout << "\x1b[38;5;214m" << field << "\x1b[0m" << endl;
                    } else{
                        cout << setw(3) << ""
                             << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                        cout << "\x1b[38;5;214m\"" << field << "\"\x1b[0m" << endl;
                    }

                } else{
                    if (header.fieldsType [j] == "int" || header.fieldsType [j] == "float"){
                        cout << setw(3) << ""
                             << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                        cout << "\x1b[38;5;214m" << field << "\x1b[0m," << endl;
                    } else{
                        cout << setw(3) << ""
                             << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                        cout << "\x1b[38;5;214m\"" << field << "\"\x1b[0m," << endl;
                    }
                }

                j++;
            }
            if (i == static_cast<int>(header.cantRegistros) - 1)
                cout << setw(1) << ""
                     << "\x1b[38;5;21m}\x1b[0m" << endl;
            else
                cout << setw(1) << ""
                     << "\x1b[38;5;21m}\x1b[0m," << endl;
        }
    }
    cout << "\x1b[38;5;201m]\x1b[0m" << endl;
    file.close();
}

void ReadFunction::SearchPK(const string& fileName, const string& primaryKey, bool isNumber, bool opcionSecreta){
    try{
        Header header = ReadBinaryHeader(fileName);
        regex rgxNumber("(^[0-9]+$)|(^[0-9]+\\.[0-9]+$)");
        vector<Index> primaryKeys;
        vector<int> keysIndex;
        if (header.fieldsName.empty()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        ifstream file(fileName, ios::in | ios::binary);
        if (!file.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        // Abrir archivo indice
        string indexFileName = fileName.substr(0, fileName.size() - 4) + ".idx";
        ifstream indexFile(indexFileName, ios::in | ios::binary);
        if (!indexFile.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Index file not found\" \n");
        }
        // Identificar la llave primaria en el header
        int primaryKeyIndex = 0;
        for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
            if (header.fieldsName [i] == header.primaryKey){
                primaryKeyIndex = i;
                break;
            }
        }
        int sizePrimaryKey = header.fieldsSize [primaryKeyIndex];
        for (int i = 0; i < static_cast<int>(header.cantRegistros); i++){
            Index index;
            char bufferPrimaryKey [sizePrimaryKey];
            indexFile.read(bufferPrimaryKey, sizePrimaryKey);
            int offset;
            indexFile.read(reinterpret_cast<char*>(&offset), sizeof(int));
            string primaryKey(bufferPrimaryKey);
            primaryKey = primaryKey.substr(0, sizePrimaryKey);
            index.key = primaryKey;
            index.offset = offset;
            primaryKeys.push_back(index);
        }
        int searchoffset = -1;
        if (isNumber){
            if (!regex_match(primaryKeys [0].key, rgxNumber)){
                throw runtime_error(
                        "\x1b[38;5;21m{\x1b[38;5;117m\n"
                        "  \"result\": \x1b[38;5;197m\"not found\"\x1b[0m\n"
                        "\x1b[38;5;21m}\x1b[0m\n");
            }
            sort(primaryKeys.begin(), primaryKeys.end(), CompareKeys);
            Index searchIndex {primaryKey, 0};
            auto it = lower_bound(primaryKeys.begin(), primaryKeys.end(), searchIndex, CompareKeys);
            searchoffset = (it != primaryKeys.end() && it->key == primaryKey) ? it->offset : -1;

        } else{
            if (regex_match(primaryKeys [1].key, rgxNumber)){
                throw runtime_error(
                        "\x1b[38;5;21m{\x1b[38;5;117m\n"
                        "  \"result\": \x1b[38;5;197m\"not found\"\x1b[0m\n"
                        "\x1b[38;5;21m}\x1b[0m\n");
            }
            sort(primaryKeys.begin(), primaryKeys.end(), CompareKeysString);
            Index searchIndex {primaryKey, 0};
            auto it = lower_bound(primaryKeys.begin(), primaryKeys.end(), searchIndex, CompareKeysString);
            searchoffset = (it != primaryKeys.end() && it->key == primaryKey) ? it->offset : -1;
        }
        if (searchoffset == -1){
            throw runtime_error(
                    "\x1b[38;5;21m{\x1b[38;5;117m\n"
                    "  \"result\": \x1b[38;5;197m\"not found\"\x1b[0m\n"
                    "\x1b[38;5;21m}\x1b[0m\n");
        } else{
            if (opcionSecreta){
                file.seekg(searchoffset);
                int sizeRecord = 0;
                for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
                    sizeRecord += header.fieldsSize [i];
                }
                char bufferRecord [sizeRecord];
                file.read(bufferRecord, sizeRecord);
                string record(bufferRecord);
                string field;
                stringstream ss(record);
                int j = 0;
                cout << setw(1) << ""
                     << "\x1b[38;5;21m{\x1b[0m" << endl;
                while (getline(ss, field, ',')){
                    if (j == static_cast<int>(header.fieldsName.size()) - 1){
                        if (header.fieldsType [j] == "float" || header.fieldsType [j] == "int"){
                            cout << setw(3) << ""
                                 << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                            cout << "\x1b[38;5;214m" << field << "\x1b[0m" << endl;
                        } else{
                            cout << setw(3) << ""
                                 << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                            cout << "\x1b[38;5;214m\"" << field << "\"\x1b[0m" << endl;
                        }

                    } else{
                        if (header.fieldsType [j] == "float" || header.fieldsType [j] == "int"){
                            cout << setw(3) << ""
                                 << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                            cout << "\x1b[38;5;214m" << field << "\x1b[0m," << endl;
                        } else{
                            cout << setw(3) << ""
                                 << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                            cout << "\x1b[38;5;214m\"" << field << "\"\x1b[0m," << endl;
                        }
                    }

                    j++;
                }
                cout << setw(1) << ""
                     << "\x1b[38;5;21m},\x1b[0m";
            } else{
                file.seekg(searchoffset);
                int sizeRecord = 0;
                for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
                    sizeRecord += header.fieldsSize [i];
                }
                char bufferRecord [sizeRecord];
                file.read(bufferRecord, sizeRecord);
                string record(bufferRecord);
                string field;
                stringstream ss(record);
                int j = 0;
                cout << setw(1) << ""
                     << "\x1b[38;5;21m{\x1b[0m" << endl;
                while (getline(ss, field, ',')){
                    if (j == static_cast<int>(header.fieldsName.size()) - 1){
                        if (header.fieldsType [j] == "float" || header.fieldsType [j] == "int"){
                            cout << setw(3) << ""
                                 << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                            cout << "\x1b[38;5;214m" << field << "\x1b[0m" << endl;
                        } else{
                            cout << setw(3) << ""
                                 << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                            cout << "\x1b[38;5;214m\"" << field << "\"\x1b[0m" << endl;
                        }

                    } else{
                        if (header.fieldsType [j] == "float" || header.fieldsType [j] == "int"){
                            cout << setw(3) << ""
                                 << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                            cout << "\x1b[38;5;214m" << field << "\x1b[0m," << endl;
                        } else{
                            cout << setw(3) << ""
                                 << "\x1b[38;5;117m\"" << header.fieldsName [j] << "\": \x1b[0m";
                            cout << "\x1b[38;5;214m\"" << field << "\"\x1b[0m," << endl;
                        }
                    }

                    j++;
                }
                cout << setw(1) << ""
                     << "\x1b[38;5;21m}\x1b[0m" << endl;
            }
        }

        file.close();
        indexFile.close();
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

bool ReadFunction::CompareKeys(const Index& a, const Index& b){
    return stoi(a.key) < stoi(b.key);
}

bool ReadFunction::CompareKeysString(const Index& key1, const Index& key2){
    return key1.key < key2.key;
}

void ReadFunction::searchSDK(const string& fileName, const string& secondaryKey, const string& valueKey){
    try{
        vector<SDKIndex> listSdkIndex;
        regex rgxNumber("(^[0-9]+$)|(^[0-9]+\\.[0-9]+$)");
        ifstream file(fileName, ios::in | ios::binary);
        if (!file.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        // Abrir indice secundario
        string sdkfileName = fileName.substr(0, fileName.size() - 4) + "-" + secondaryKey + ".sdx";
        string idxFileName = fileName.substr(0, fileName.size() - 4) + ".idx";
        ifstream sdkFile(sdkfileName, ios::in | ios::binary);
        if (!sdkFile.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Secondary index file not found\" \n");
        }
        Header header = ReadBinaryHeader(fileName);
        int sizePrimaryKey = -1;
        int sizeSecondaryKey = -1;
        for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
            if (header.fieldsName [i] == header.primaryKey){
                sizePrimaryKey = header.fieldsSize [i];
            }
            if (header.fieldsName [i] == secondaryKey){
                sizeSecondaryKey = header.fieldsSize [i];
            }
        }
        if (sizePrimaryKey == -1 || sizeSecondaryKey == -1){
            throw runtime_error(
                    "\x1b[38;5;13m[]\x1b[0m\n");
        }
        for (int i = 0; i < header.cantRegistros; i++){
            SDKIndex sdkIndex;
            char bufferPrimaryKey [sizePrimaryKey];
            char bufferSecondaryKey [sizeSecondaryKey];
            sdkFile.read(bufferPrimaryKey, sizePrimaryKey);
            sdkFile.read(bufferSecondaryKey, sizeSecondaryKey);
            string primaryKey(bufferPrimaryKey);
            string secondaryKey2(bufferSecondaryKey);
            primaryKey = primaryKey.substr(0, sizePrimaryKey);
            sdkIndex.primaryKey = primaryKey;
            sdkIndex.secondaryKey = secondaryKey2;
            listSdkIndex.push_back(sdkIndex);
        }
        cout << "\x1b[38;5;13m[\x1b[0m" << endl;
        bool isValid = false;
        for (const auto& sdk : listSdkIndex){
            if (sizeSecondaryKey == 1){
                if (sdk.secondaryKey [0] == valueKey [0]){
                    if (regex_match(sdk.primaryKey, rgxNumber)){
                        SearchPK(fileName, sdk.primaryKey, true, true);
                        cout << endl;
                        isValid = true;
                    } else{
                        SearchPK(fileName, sdk.primaryKey, false, true);
                        isValid = true;
                        cout << endl;
                    }
                }
            } else{
                if (sdk.secondaryKey == valueKey){
                    if (regex_match(sdk.primaryKey, rgxNumber)){
                        SearchPK(fileName, sdk.primaryKey, true, true);
                        isValid = true;
                        cout << endl;
                    } else{
                        SearchPK(fileName, sdk.primaryKey, false, true);
                        isValid = true;
                        cout << endl;
                    }
                }
            }
        }
        if (isValid){
            cout << "\033[1A";  // Mueve el cursos arriba
            cout << "\033[2K";  // Limpia la linea
            cout << setw(1) << ""
                 << "\x1b[38;5;21m}\x1b[0m\n";
            cout << "\x1b[38;5;13m]\x1b[0m\n";
        } else{
            cout << "\x1b[38;5;13m]\x1b[0m\n";
        }

        file.close();
        sdkFile.close();
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}
void ReadFunction::SearchOffset(const string& fileName, const string& primaryKey, bool isNumber, string& listFields, string& newKey){
    try{
        Header header = ReadBinaryHeader(fileName);
        regex rgxNumber("(^[0-9]+$)|(^[0-9]+\\.[0-9]+$)");
        vector<Index> primaryKeys;
        vector<string> listKeys;
        int sizeRecords = 0;
        if (header.fieldsName.empty()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
            sizeRecords += header.fieldsSize [i];
        }
        fstream file(fileName, ios::in | ios::out | ios::binary);
        if (!file.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        // Abrir archivo indice
        string indexFileName = fileName.substr(0, fileName.size() - 4) + ".idx";
        fstream indexFile(indexFileName, ios::in | ios::out | ios::binary);
        if (!indexFile.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Index file not found\" \n");
        }
        // Identificar la llave primaria en el header
        int primaryKeyIndex = 0;
        for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
            if (header.fieldsName [i] == header.primaryKey){
                primaryKeyIndex = i;
                break;
            }
        }
        int sizePrimaryKey = header.fieldsSize [primaryKeyIndex];
        for (int i = 0; i < static_cast<int>(header.cantRegistros); i++){
            Index index;
            char bufferPrimaryKey [sizePrimaryKey];
            indexFile.read(bufferPrimaryKey, sizePrimaryKey);
            int offset;
            indexFile.read(reinterpret_cast<char*>(&offset), sizeof(int));
            string primaryKey(bufferPrimaryKey);
            primaryKey = primaryKey.substr(0, sizePrimaryKey);
            index.key = primaryKey;
            index.offset = offset;
            primaryKeys.push_back(index);
        }
        int searchoffset = -1;
        if (isNumber){
            if (!regex_match(primaryKeys [0].key, rgxNumber)){
                throw runtime_error(
                        "\x1b[38;5;21m{\x1b[38;5;117m\n"
                        "  \"result\": \x1b[38;5;197m\"not found\"\x1b[0m\n"
                        "\x1b[38;5;21m}\x1b[0m\n");
            }
            sort(primaryKeys.begin(), primaryKeys.end(), CompareKeys);
            Index searchIndex {primaryKey, 0};
            auto it = lower_bound(primaryKeys.begin(), primaryKeys.end(), searchIndex, CompareKeys);
            searchoffset = (it != primaryKeys.end() && it->key == primaryKey) ? it->offset : -1;

        } else{
            if (regex_match(primaryKeys [1].key, rgxNumber)){
                throw runtime_error(
                        "\x1b[38;5;21m{\x1b[38;5;117m\n"
                        "  \"result\": \x1b[38;5;197m\"not found\"\x1b[0m\n"
                        "\x1b[38;5;21m}\x1b[0m\n");
            }
            sort(primaryKeys.begin(), primaryKeys.end(), CompareKeysString);
            Index searchIndex {primaryKey, 0};
            auto it = lower_bound(primaryKeys.begin(), primaryKeys.end(), searchIndex, CompareKeysString);
            searchoffset = (it != primaryKeys.end() && it->key == primaryKey) ? it->offset : -1;
        }
        if (searchoffset == -1){
            throw runtime_error(
                    "\x1b[38;5;21m{\x1b[38;5;117m\n"
                    "  \"result\": \x1b[38;5;197m\"not found\"\x1b[0m\n"
                    "\x1b[38;5;21m}\x1b[0m\n");
        }
        int posPrimaryKey = -1;
        for (int i = 0; i < static_cast<int>(primaryKeys.size()); i++){
            if (primaryKeys [i].key == primaryKey){
                posPrimaryKey = i;
            }
            listKeys.push_back(primaryKeys [i].key);
        }
        // Validar que la llave primaria exista
        bool isPrimaryKey = false;
        for (int i = 0; i < static_cast<int>(listKeys.size()); i++){
            if (listKeys [i] == primaryKey){
                isPrimaryKey = true;
            }
        }
        if (!isPrimaryKey){
            throw runtime_error(
                    "\x1b[38;5;21m{\x1b[38;5;117m\n"
                    "  \"result\": \x1b[38;5;197m\"not found\"\x1b[0m\n"
                    "\x1b[38;5;21m}\x1b[0m\n");
        }
        listKeys.erase(listKeys.begin() + posPrimaryKey);
        if (!UniqueIDMethod(listKeys, newKey)){
            throw runtime_error(
                    "\x1b[38;5;21m{\x1b[38;5;117m\n"
                    "  \"result\": \x1b[38;5;197m\"Error, Primary Key is not unique\"\x1b[0m\n"
                    "\x1b[38;5;21m}\x1b[0m\n");
        } else{
            // modificar el registro
            file.seekp(searchoffset);
            file.write(listFields.c_str(), sizeRecords);
        }
        // Ahora modificar los indices secundarios
        int sizeSecondaryKey = 0;
        int posSecondary;
        for (int i = 0; i < static_cast<int>(header.secondaryKey.size()); i++){
            for (int j = 0; j < static_cast<int>(header.fieldsName.size()); j++){
                if (header.fieldsName [j] == header.secondaryKey [i]){
                    sizeSecondaryKey = header.fieldsSize [j];
                    posSecondary = j;
                    break;
                }
            }
            int contTemp = 0;
            stringstream ss(listFields);
            string temp;
            string newSecondaryKey;
            while (getline(ss, temp, ',')){
                if (contTemp == posSecondary){
                    newSecondaryKey = temp;
                }
                contTemp++;
            }
            string nameSecondary = fileName.substr(0, fileName.size() - 4) + "-" + header.secondaryKey [i] + ".sdx";
            WriteFunction::ModSdx(nameSecondary, newKey, sizeSecondaryKey, primaryKey, newSecondaryKey);
        }
        // modificar el archivo de indice primario
        indexFile.seekp(0, ios::beg);
        for (int i = 0; i < static_cast<int>(primaryKeys.size()); i++){
            if (primaryKeys [i].key == primaryKey){
                primaryKeys [i].key = newKey;
                indexFile.write(primaryKeys [i].key.c_str(), sizePrimaryKey);
                indexFile.write(reinterpret_cast<char*>(&primaryKeys [i].offset), sizeof(int));
            } else{
                indexFile.write(primaryKeys [i].key.c_str(), sizePrimaryKey);
                indexFile.write(reinterpret_cast<char*>(&primaryKeys [i].offset), sizeof(int));
            }
        }
        file.close();
        indexFile.close();
        cout << "\x1b[38;5;21m{\x1b[38;5;117m\n"
                "  \"result\": \x1b[38;5;46m\"OK\"\x1b[0m\n"
                "\x1b[38;5;21m}\x1b[0m\n";
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

vector<string> ReadFunction::ReadBinaryRecordsKey(const string& fileName, int& sizeRecords, int& sizeKey){
    vector<string> listKeys;
    try{
        ifstream file(fileName, ios::in | ios::binary);
        if (!file.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"File not found\" \n");
        }
        for (int i = 0; i < sizeRecords; i++){
            char bufferKey [sizeKey];
            file.read(bufferKey, sizeKey);
            string key(bufferKey);
            int offset;
            file.read(reinterpret_cast<char*>(&offset), sizeof(int));
            listKeys.push_back(key);
        }
        file.close();
        return listKeys;
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
        return listKeys;
    }
}