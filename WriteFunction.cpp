#include "WriteFunction.h"
void WriteFunction::CreateFile(const string& fileName){
    try{
        // Abre el archivo json y crear archivos necesarios
        string nameBinary = fileName.substr(0, fileName.find_last_of("."));
        string finalSecondaryIndexFile;
        ifstream file(fileName);

        if (!file.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"json file not found\"} \n");
        }
        // Usar libreria json para leer el archivo json
        Header headerJson;
        json jData;
        file >> jData;
        // Obtener los campos
        json jFields = jData ["fields"];
        // Valida que el json tenga campos o el primary key no este vacio
        if (jFields.empty()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"fields has space or it's in an incorrect form\"} \n");
        }
        if (jData ["primary-key"].empty()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"primary index field does not exist\"} \n");
        }
        if (jData ["secondary-key"].empty()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"secondary index field does not exist\"} \n");
        }
        // Obtener el nombre de los campos
        for (auto& element : jFields){
            // Valida si el campo no tiene espacios
            if (element ["name"].empty()){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"name is empty or is an invalid format\"} \n");
            }
            string name = element ["name"];
            if (name.find(" ") != string::npos || name == ""){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"field contains space or is empty\"} \n");
            }
            headerJson.fieldsName.push_back(element ["name"]);
        }
        // Obtener el tipo de los campos
        for (auto& element : jFields){
            string type = element ["type"];
            if (element ["type"].empty()){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"type is empty or is an invalid format\"} \n");
            }
            if (type != "char" && type != "int" && type != "float"){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"type is not valid\"} \n");
            }
            headerJson.fieldsType.push_back(element ["type"]);
        }
        // Obtener el tamaño de los campos
        for (auto& element : jFields){
            if (element ["length"].empty()){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"length is empty or is an invalid format\"} \n");
            }
            if (!element ["length"].is_number()){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"length is not valid\"} \n");
            }
            headerJson.fieldsSize.push_back(element ["length"]);
        }
        // Obtener el nombre de la llave primaria
        string pk = jData ["primary-key"];
        if (pk.find(" ") != string::npos || pk == ""){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"primary key contains space, or is empty\"} \n");
        }
        headerJson.primaryKey = jData ["primary-key"];
        // Obtener las llaves secundarias
        for (auto& element : jData ["secondary-key"]){
            string sk = element;
            if (sk.find(" ") != string::npos || sk == ""){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"secondary key contains space or is empty\"} \n");
            }
            headerJson.secondaryKey.push_back(element);
            // Crear archivos secundarios
            finalSecondaryIndexFile += "\"" + nameBinary.substr(nameBinary.find_last_of("/") + 1) + "-" + sk + ".sdx\", ";
            ofstream fileSecondary(nameBinary + "-" + sk + ".sdx");
            if (!fileSecondary.is_open()){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"secondary index file could not be created\"} \n");
            }
        }
        finalSecondaryIndexFile = finalSecondaryIndexFile.substr(0, finalSecondaryIndexFile.size() - 2);
        ofstream fileHeader(nameBinary + ".bin", std::ios::binary);
        if (!fileHeader.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"binary file could not be created\"} \n");
        }
        headerJson.refAvailList = -1;
        headerJson.cantRegistros = 0;
        WriteBinaryHeader(nameBinary + ".bin", headerJson);
        ofstream fileIndex(nameBinary + ".idx", std::ios::binary);
        if (!fileIndex.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"binary or index file could not be created\"} \n");
        }
        // cierra el archivo o los archivos
        file.close();
        fileHeader.close();
        fileIndex.close();
        cout << "{\"result\": \x1b[38;5;10m\"OK\" \x1b[0m, \"fields-count\": \x1b[38;5;226m" << jData ["fields"].size() << "\x1b[0m, \"file\": \x1b[38;5;226m\"" << nameBinary.substr(nameBinary.find_last_of("/") + 1) << ".bin\"\x1b[0m, \"index\": \x1b[38;5;226m\"" << nameBinary.substr(nameBinary.find_last_of("/") + 1) << ".idx\"\x1b[0m, \"secondary\": [\x1b[38;5;226m" << finalSecondaryIndexFile << "\x1b[0m]}\n";
    } catch (json::parse_error& e){
        cerr << "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Format not recognized\"} \n"
             << e.what() << '\n';
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void WriteFunction::WriteBinaryHeader(const string& fileName, Header& header){
    try{
        string nameBinary = fileName.substr(0, fileName.find_last_of("."));
        std::fstream fileHeader(nameBinary + ".bin", std::ios::binary | std::ios::in | std::ios::out);
        if (!fileHeader.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        // Asegurar espacio para los vectores y poder recuperarlos
        int sizeFields = header.fieldsName.size();
        int sizeSecondary = header.secondaryKey.size();
        // Escribir el header en el archivo binario
        fileHeader.seekp(0, ios::beg);
        fileHeader.write(reinterpret_cast<const char*>(&sizeFields), sizeof(int));
        fileHeader.write(reinterpret_cast<const char*>(&sizeSecondary), sizeof(int));
        // Guardar los elementos en los vectores del header
        // Guardar elementos de string, guarda cada nombre en este caso de los campos
        for (int i = 0; i < sizeFields; i++){
            int sizeFieldName = header.fieldsName [i].size();
            int sizeType = header.fieldsType [i].size();
            fileHeader.write(reinterpret_cast<const char*>(&sizeFieldName), sizeof(int));
            fileHeader.write(reinterpret_cast<const char*>(header.fieldsName [i].c_str()), sizeFieldName);
            fileHeader.write(reinterpret_cast<const char*>(&sizeType), sizeof(int));
            fileHeader.write(reinterpret_cast<const char*>(header.fieldsType [i].c_str()), sizeType);
            fileHeader.write(reinterpret_cast<const char*>(&header.fieldsSize [i]), sizeof(int));
        }
        for (int i = 0; i < sizeSecondary; i++){
            int sizeSecondary = header.secondaryKey [i].size();
            fileHeader.write(reinterpret_cast<const char*>(&sizeSecondary), sizeof(int));
            fileHeader.write(reinterpret_cast<const char*>(header.secondaryKey [i].c_str()), sizeSecondary);
        }
        // Guarda la llaveprimaria
        int sizePrimaryKey = header.primaryKey.size();
        fileHeader.write(reinterpret_cast<const char*>(&sizePrimaryKey), sizeof(int));
        fileHeader.write(reinterpret_cast<const char*>(header.primaryKey.c_str()), sizePrimaryKey);
        // Guarda la ref del Avail List
        fileHeader.write(reinterpret_cast<const char*>(&header.refAvailList), sizeof(int));
        // guarda la cantidad de registros
        fileHeader.write(reinterpret_cast<const char*>(&header.cantRegistros), sizeof(int));
        // Escribe un fin de linea para identificar el final del header
        fileHeader.write("~", 1);
        fileHeader.close();
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void WriteFunction::WriteBinaryHeaderRecords(const string& fileName, vector<string>& records, vector<int>& sizeRecords, string& primaryKey, int& sizePrimaryKey, Header& header){
    try{
        string nameIndex = fileName.substr(0, fileName.size() - 4) + ".idx";
        string record;
        int sizeRecord = 0;
        std::fstream file(fileName, ios::binary | ios::app);
        if (!file.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        } else{
            file.seekp(0, ios::end);
            for (int i = 0; i < static_cast<int>(records.size()); i++){
                record += records [i] + ",";
                sizeRecord += sizeRecords [i];
            }
            record = record.substr(0, record.size() - 1);
            int offset = file.tellp();
            WriteIndex(nameIndex, primaryKey, offset, sizePrimaryKey);
            int sizeSecondary;
            for (const auto& secondary : header.secondaryKey){
                for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
                    if (header.fieldsName [i] == secondary){
                        sizeSecondary = header.fieldsSize [i];
                        break;
                    }
                }
                int j = 0;
                stringstream ss(record);
                string field;
                while (getline(ss, field, ',')){
                    if (header.fieldsName [j] == secondary){
                        string nameSdx = fileName.substr(0, fileName.size() - 4) + "-" + secondary + ".sdx";
                        WriteSecondaryIndex(nameSdx, primaryKey, field, sizePrimaryKey, sizeSecondary);
                        break;
                    }
                    j++;
                }
            }
            file.write(record.c_str(), sizeRecord);
            file.close();
        }
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void WriteFunction::WriteIndex(const string& fileName, string& key, int& offset, int& sizeKey){
    try{
        std::fstream file(fileName, ios::binary | ios::in | ios::out);
        if (!file.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        } else{
            file.seekp(0, ios::end);
            file.write(key.c_str(), sizeKey);
            file.write(reinterpret_cast<char*>(&offset), sizeof(offset));
            file.close();
        }
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void WriteFunction::WriteSecondaryIndex(const string& fileName, string& refPrimaryKey, const string& secondaryKey, int& sizeKey, int& sizeSecondaryKey){
    try{
        std::fstream file(fileName, ios::binary | ios::in | ios::out);
        if (!file.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        } else{
            file.seekp(0, ios::end);
            file.write(refPrimaryKey.c_str(), sizeKey);
            file.write(secondaryKey.c_str(), sizeSecondaryKey);
            file.close();
        }
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void WriteFunction::DeleteRecord(const string& fileName, const string& primaryKey, bool isNumber){
    try{
        vector<Index> listIndex;
        vector<SDKIndex> listSdkIndex;
        int posEliminar = -1;
        regex rgxNumber("(^[0-9]+$)|(^[0-9]+\\.[0-9]+$)");
        Header header = ReadFunction::ReadBinaryHeader(fileName);
        if (header.fieldsName.empty()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"File is empty\" \n");
        }
        string idxFileName = fileName.substr(0, fileName.size() - 4) + ".idx";
        fstream indexFile(idxFileName, ios::in | ios::binary);
        if (!indexFile.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Index file not found\" \n");
        }
        int sizePrimaryKey = -1;
        for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
            if (header.fieldsName [i] == header.primaryKey){
                sizePrimaryKey = header.fieldsSize [i];
            }
        }
        for (int i = 0; i < header.cantRegistros; i++){
            Index index;
            char bufferPrimaryKey [sizePrimaryKey];
            indexFile.read(bufferPrimaryKey, sizePrimaryKey);
            string primaryKey2(bufferPrimaryKey);
            primaryKey2 = primaryKey2.substr(0, sizePrimaryKey);
            index.key = primaryKey2;
            indexFile.read(reinterpret_cast<char*>(&index.offset), sizeof(int));
            listIndex.push_back(index);
        }
        indexFile.close();
        int searchoffset = 0;
        if (isNumber){
            if (!regex_match(listIndex [0].key, rgxNumber)){
                throw runtime_error(
                        "\x1b[38;5;21m{\x1b[38;5;117m\n"
                        "  \"result\": \x1b[38;5;197m\"not found\"\x1b[0m\n"
                        "\x1b[38;5;21m}\x1b[0m\n");
            }
            sort(listIndex.begin(), listIndex.end(), ReadFunction::CompareKeys);
            Index searchIndex {primaryKey, 0};
            auto it = lower_bound(listIndex.begin(), listIndex.end(), searchIndex, ReadFunction::CompareKeys);
            searchoffset = (it != listIndex.end() && it->key == primaryKey) ? it->offset : -1;
            posEliminar = it - listIndex.begin();

        } else{
            if (regex_match(listIndex [1].key, rgxNumber)){
                throw runtime_error(
                        "\x1b[38;5;21m{\x1b[38;5;117m\n"
                        "  \"result\": \x1b[38;5;197m\"not found\"\x1b[0m\n"
                        "\x1b[38;5;21m}\x1b[0m\n");
            }
            sort(listIndex.begin(), listIndex.end(), ReadFunction::CompareKeysString);
            Index searchIndex {primaryKey, 0};
            auto it = lower_bound(listIndex.begin(), listIndex.end(), searchIndex, ReadFunction::CompareKeysString);
            searchoffset = (it != listIndex.end() && it->key == primaryKey) ? it->offset : -1;
            posEliminar = it - listIndex.begin();
        }
        if (searchoffset == -1){
            throw runtime_error(
                    "\x1b[38;5;21m{\x1b[38;5;117m\n"
                    "  \"result\": \x1b[38;5;197m\"not found\"\x1b[0m\n"
                    "\x1b[38;5;21m}\x1b[0m\n");
        }
        fstream file(fileName, ios::in | ios::out | ios::binary);
        if (!file.is_open()){
            throw runtime_error(
                    "\x1b[38;5;21m{\x1b[38;5;117m\n"
                    "  \"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m\n"
                    "\x1b[38;5;21m}\x1b[0m\n");
        }
        int sizeRecord = 0;
        for (int i = 0; i < static_cast<int>(header.fieldsSize.size()); i++){
            sizeRecord += header.fieldsSize [i];
        }
        // Va sobreescribir el registro con *offset ademas necesitamos actualizar el availList
        if (header.refAvailList != -1){
            file.seekp(searchoffset);
            string deleteRecord = "*" + to_string(header.refAvailList);
            file.write(deleteRecord.c_str(), sizeRecord);
            header.refAvailList = searchoffset;
        } else{
            file.seekp(searchoffset);
            string deleteRecord = "*" + to_string(-1);
            file.write(deleteRecord.c_str(), sizeRecord);
            header.refAvailList = searchoffset;
        }
        header.cantRegistros--;
        WriteBinaryHeader(fileName, header);
        string primaryKeyDelete = listIndex [posEliminar].key;
        int listIndezAnterior = listIndex.size();
        listIndex.erase(listIndex.begin() + posEliminar);
        ofstream indexFile2(idxFileName, ios::binary);
        if (!indexFile2.is_open()){
            throw runtime_error(
                    "\x1b[38;5;21m{\x1b[38;5;117m\n"
                    "  \"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m\n"
                    "\x1b[38;5;21m}\x1b[0m\n");
        }
        indexFile2.seekp(0, ios::beg);
        for (int i = 0; i < static_cast<int>(listIndex.size()); i++){
            indexFile2.write(listIndex [i].key.c_str(), sizePrimaryKey);
            indexFile2.write(reinterpret_cast<char*>(&listIndex [i].offset), sizeof(int));
            indexFile2.flush();
        }
        // Eliminar en el indice secundario
        for (int i = 0; i < static_cast<int>(header.secondaryKey.size()); i++){
            int sizeSecondaryKey = 0;
            for (int j = 0; j < static_cast<int>(header.fieldsName.size()); j++){
                if (header.fieldsName [j] == header.secondaryKey [i]){
                    sizeSecondaryKey = header.fieldsSize [j];
                    break;
                }
            }
            string nameSecondary = fileName.substr(0, fileName.size() - 4) + "-" + header.secondaryKey [i] + ".sdx";
            ifstream secondaryIndexFile(nameSecondary, ios::in | ios::binary);
            if (!secondaryIndexFile.is_open()){
                throw runtime_error(
                        "\x1b[38;5;21m{\x1b[38;5;117m\n"
                        "  \"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m\n"
                        "\x1b[38;5;21m}\x1b[0m\n");
            }
            listSdkIndex.clear();
            for (int j = 0; j < listIndezAnterior; j++){
                SDKIndex indexSdx;
                char bufferPrimaryKey [sizePrimaryKey];
                secondaryIndexFile.read(bufferPrimaryKey, sizePrimaryKey);
                char bufferSecondaryKey [sizeSecondaryKey];
                secondaryIndexFile.read(bufferSecondaryKey, sizeSecondaryKey);
                string primaryKey(bufferPrimaryKey);
                primaryKey = primaryKey.substr(0, sizePrimaryKey);
                string secondaryKey(bufferSecondaryKey);
                secondaryKey = secondaryKey.substr(0, sizeSecondaryKey);
                if (sizePrimaryKey == 1){
                    indexSdx.primaryKey = primaryKey [0];
                } else{
                    indexSdx.primaryKey = primaryKey;
                }
                if (sizeSecondaryKey == 1){
                    indexSdx.secondaryKey = secondaryKey [0];
                } else{
                    indexSdx.secondaryKey = secondaryKey;
                }
                if (indexSdx.primaryKey != primaryKeyDelete){
                    listSdkIndex.push_back(indexSdx);
                } else{
                }
            }
            secondaryIndexFile.close();
            ofstream secondaryIndexFile2(nameSecondary, ios::binary);
            if (!secondaryIndexFile2.is_open()){
                throw runtime_error(
                        "\x1b[38;5;21m{\x1b[38;5;117m\n"
                        "  \"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m\n"
                        "\x1b[38;5;21m}\x1b[0m\n");
            }
            secondaryIndexFile2.seekp(0, ios::beg);
            for (int j = 0; j < static_cast<int>(listSdkIndex.size()); j++){
                secondaryIndexFile2.write(listSdkIndex [j].primaryKey.c_str(), sizePrimaryKey);
                secondaryIndexFile2.write(listSdkIndex [j].secondaryKey.c_str(), sizeSecondaryKey);
            }
            secondaryIndexFile2.close();
        }
        file.close();
        indexFile2.close();
        cout << "\x1b[38;5;21m{\x1b[38;5;117m"
             << "  \"result\": \x1b[38;5;82m\"OK\"\x1b[0m"
             << "\x1b[38;5;21m}\x1b[0m\n";
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void WriteFunction::ModRecord(const string& fileName, const string& primaryKey, bool isNumber, const string& data){
    try{
        Header header = ReadFunction::ReadBinaryHeader(fileName);
        vector<string> listRecordSplit;
        regex rgxNumber("(^[0-9]+$)|(^[0-9]+\\.[0-9]+$)");
        string newKey;
        if (header.fieldsName.empty()){
            throw runtime_error(
                    "\x1b[38;5;21m{\x1b[38;5;117m\n"
                    "  \"result\": \x1b[38;5;197m\"Binary is empty\"\x1b[0m\n"
                    "\x1b[38;5;21m}\x1b[0m\n");
        }
        int posPrimaryKey = 0;
        int sizePrimaryKey = 0;
        vector<int> posSecondaryKey;
        for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
            if (header.fieldsName [i] == header.primaryKey){
                posPrimaryKey = i;
                sizePrimaryKey = header.fieldsSize [i];
            }
            for (int j = 0; j < static_cast<int>(header.secondaryKey.size()); j++){
                if (header.fieldsName [i] == header.secondaryKey [j]){
                    posSecondaryKey.push_back(i);
                }
            }
        }
        // Validar los campos de la data
        stringstream ss(data);
        string field;
        int posHeader = 0;
        while (getline(ss, field, ',')){
            if (posHeader > static_cast<int>(header.fieldsName.size())){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Field type does not match\" \n");
            }
            stringstream ss2(field);
            string nameField;
            getline(ss2, nameField, ':');
            if (header.fieldsName [posHeader] == nameField){
                listRecordSplit.push_back(field.substr(nameField.size() + 1));
            } else{
                throw runtime_error(
                        "\x1b[38;5;21m{\x1b[38;5;117m\n"
                        "  \"result\": \x1b[38;5;197m\"Error, Invalid Data record for this file\"\x1b[0m\n"
                        "\x1b[38;5;21m}\x1b[0m\n");
            }
            if (header.fieldsType [posHeader] == "int" || header.fieldsType [posHeader] == "float"){
                if (field.substr(nameField.size() + 1).find(' ') != string::npos){
                    throw runtime_error(
                            "\x1b[38;5;21m{\x1b[38;5;117m\n"
                            "  \"result\": \x1b[38;5;197m\"Error, Invalid Data record for this file, Number with space?\"\x1b[0m\n"
                            "\x1b[38;5;21m}\x1b[0m\n");
                }
            }
            if (posHeader == posPrimaryKey){
                if (field.substr(nameField.size() + 1) == ""){
                    throw runtime_error(
                            "\x1b[38;5;21m{\x1b[38;5;117m\n"
                            "  \"result\": \x1b[38;5;197m\"Error, Primary Key can't be empty\"\x1b[0m\n"
                            "\x1b[38;5;21m}\x1b[0m\n");
                } else if (isNumber && !regex_match(field.substr(nameField.size() + 1), rgxNumber)){
                    throw runtime_error(
                            "\x1b[38;5;21m{\x1b[38;5;117m\n"
                            "  \"result\": \x1b[38;5;197m\"Error, Primary Key is not a number\"\x1b[0m\n"
                            "\x1b[38;5;21m}\x1b[0m\n");
                } else if (static_cast<int>(field.substr(nameField.size() + 1).size()) > sizePrimaryKey){
                    throw runtime_error(
                            "\x1b[38;5;21m{\x1b[38;5;117m\n"
                            "  \"result\": \x1b[38;5;197m\"Error, Primary Key is too long\"\x1b[0m\n"
                            "\x1b[38;5;21m}\x1b[0m\n");
                } else{
                    newKey = field.substr(nameField.size() + 1);
                }
            }
            for (int i = 0; i < static_cast<int>(posSecondaryKey.size()); i++){
                if (posHeader == posSecondaryKey [i]){
                    if (field.substr(nameField.size() + 1) == ""){
                        throw runtime_error(
                                "\x1b[38;5;21m{\x1b[38;5;117m\n"
                                "  \"result\": \x1b[38;5;197m\"Error, Secondary Key can't be empty\"\x1b[0m\n"
                                "\x1b[38;5;21m}\x1b[0m\n");
                    } else if (static_cast<int>(field.substr(nameField.size() + 1).size()) > header.fieldsSize [posHeader]){
                        throw runtime_error(
                                "\x1b[38;5;21m{\x1b[38;5;117m\n"
                                "  \"result\": \x1b[38;5;197m\"Error, Secondary Key is too long\"\x1b[0m\n"
                                "\x1b[38;5;21m}\x1b[0m\n");
                    }
                }
            }
            posHeader++;
        }
        // meter del listRecordSplit a listfields
        string listFields;
        for (int i = 0; i < static_cast<int>(listRecordSplit.size()); i++){
            listFields += listRecordSplit [i] + ",";
        }
        listFields = listFields.substr(0, listFields.size() - 1);
        ReadFunction::SearchOffset(fileName, primaryKey, isNumber, listFields, newKey);
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void WriteFunction::ModSdx(const string& fileName, const string& keyMod, int& sizeSecondaryKey, const string& searchKey, string& newSecondaryKey){
    try{
        string nameBinary = fileName.substr(0, fileName.find_last_of("-")) + ".bin";
        string nameSdx = fileName.substr(fileName.find_last_of("-") + 1, fileName.size() - 4);
        nameSdx = nameSdx.substr(0, nameSdx.size() - 4);
        Header header = ReadFunction::ReadBinaryHeader(nameBinary);
        fstream file(fileName, ios::in | ios::out | ios::binary);
        if (!file.is_open()){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Binary file not found\" \n");
        }
        int sizePrimaryKey = 0;
        for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
            if (header.fieldsName [i] == header.primaryKey){
                sizePrimaryKey = header.fieldsSize [i];
                break;
            }
        }
        vector<SDKIndex> listSecondary;
        file.seekg(0, ios::beg);
        for (int i = 0; i < static_cast<int>(header.cantRegistros); i++){
            SDKIndex sdxIndex;
            char bufferPrimaryKey [sizePrimaryKey];
            char bufferSecondaryKey [sizeSecondaryKey];
            file.read(bufferPrimaryKey, sizePrimaryKey);
            file.read(bufferSecondaryKey, sizeSecondaryKey);
            string primaryKey(bufferPrimaryKey);
            string secondaryKey(bufferSecondaryKey);
            sdxIndex.primaryKey = primaryKey;
            sdxIndex.secondaryKey = secondaryKey;
            listSecondary.push_back(sdxIndex);
        }
        file.seekp(0, ios::beg);
        for (int i = 0; i < static_cast<int>(listSecondary.size()); i++){
            if (listSecondary [i].primaryKey == searchKey){
                listSecondary [i].primaryKey = keyMod;
                listSecondary [i].secondaryKey = newSecondaryKey;
                file.write(listSecondary [i].primaryKey.c_str(), sizePrimaryKey);
                file.write(listSecondary [i].secondaryKey.c_str(), sizeSecondaryKey);
            } else{
                file.write(listSecondary [i].primaryKey.c_str(), sizePrimaryKey);
                file.write(listSecondary [i].secondaryKey.c_str(), sizeSecondaryKey);
            }
        }
        file.close();
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void WriteFunction::AddRecord(const string& fileName, const string data){
    try{
        regex rgxNumber("(^[0-9]+$)|(^[0-9]+\\.[0-9]+$)");
        vector<string> listFields;
        Header header = ReadFunction::ReadBinaryHeader(fileName);
        stringstream ss(data);
        string field;
        string newKey;
        int sizeRecords = 0;
        int posPrimaryKey;
        int posSecondaryKey [header.secondaryKey.size()];
        int sizePrimaryKey;
        int sizeSecondaryKey [header.secondaryKey.size()];
        for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
            sizeRecords += header.fieldsSize [i];
            if (header.fieldsName [i] == header.primaryKey){
                posPrimaryKey = i;
                sizePrimaryKey = header.fieldsSize [i];
            }
            for (int j = 0; j < static_cast<int>(header.secondaryKey.size()); j++){
                if (header.fieldsName [i] == header.secondaryKey [j]){
                    posSecondaryKey [j] = i;
                    sizeSecondaryKey [j] = header.fieldsSize [i];
                }
            }
        }

        int contType = 0;
        while (getline(ss, field, ',')){
            // valida que no se ingresen mas campos de los que se deben
            if (contType > static_cast<int>(header.fieldsName.size())){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Field type does not match\" \n");
            }
            stringstream ss2(field);
            string fieldType;
            getline(ss2, fieldType, ':');
            string fieldValue = field.substr(field.find(":") + 1, field.size());
            // valida que el campo este en orden
            if (header.fieldsName [contType] != fieldType){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Field type does not match\" \n");
            }
            // valida que el campo no este vacio si es llave primaria o secundaria
            if (contType == posPrimaryKey){
                if (fieldValue == ""){
                    throw runtime_error(
                            "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Primary key cannot be empty\" \n");
                }
            }
            for (int i = 0; i < static_cast<int>(header.secondaryKey.size()); i++){
                if (contType == posSecondaryKey [i]){
                    if (fieldValue == ""){
                        throw runtime_error(
                                "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Secondary key cannot be empty\" \n");
                    }
                }
            }
            // Valida que si el tipo es numerico, mantenga el formato
            if (header.fieldsType [contType] == "int" || header.fieldsType [contType] == "float"){
                if (!regex_match(fieldValue, rgxNumber)){
                    throw runtime_error(
                            "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Field type does not match\" \n");
                }
            } else{
                // Valida que el tamaño del caracter no sea mayor al definido
                if (static_cast<int>(fieldValue.size()) > static_cast<int>(header.fieldsSize [contType])){
                    throw runtime_error(
                            "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Field type does not match\" \n");
                }
            }
            if (contType == posPrimaryKey){
                newKey = fieldValue;
            }
            listFields.push_back(fieldValue);
            contType++;
        }
        // Valida que la llave primaria no exista
        string nameIndex = fileName.substr(0, fileName.find(".") + 1) + "idx";
        vector<string> listKeys;
        listKeys = ReadFunction::ReadBinaryRecordsKey(nameIndex, header.cantRegistros, sizePrimaryKey);
        if (!ReadFunction::UniqueIDMethod(listKeys, newKey)){
            throw runtime_error(
                    "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"Primary key already exists\"} \n");
        }
        // Ver si hay espacio en el avail List;
        string fieldsString;
        for (int i = 0; i < static_cast<int>(listFields.size()); i++){
            fieldsString += listFields [i] + ",";
        }
        fieldsString = fieldsString.substr(0, fieldsString.size() - 1);
        if (header.refAvailList != -1){
            fstream file(fileName, ios::out | ios::binary | ios::in);
            if (!file.is_open()){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"File not found\" \n");
            }
            // Leer hasta encontrar el delimador del header
            bool foundEnd = false;
            int offset = header.refAvailList;
            int lastOffset;
            int cantEncuentros = 0;
            int changeOffset = -1;
            do{
                file.seekp(offset);
                char buffer [sizeRecords];
                file.read(buffer, sizeRecords);
                string record(buffer);
                lastOffset = offset;
                if (record [0] == '*'){
                    offset = stoi(record.substr(1));
                }
                if (offset == -1){
                    file.seekp(lastOffset);
                    file.write(fieldsString.c_str(), sizeRecords);
                    file.flush();
                    // Agregar al indice
                    WriteIndex(nameIndex, newKey, lastOffset, sizePrimaryKey);
                    // Agregar al archivo de indices secundarios
                    for (int i = 0; i < static_cast<int>(header.secondaryKey.size()); i++){
                        string nameIndexSecondary = fileName.substr(0, fileName.find(".")) + "-" + header.secondaryKey [i] + ".sdx";
                        string secondaryKey = listFields [posSecondaryKey [i]];
                        WriteSecondaryIndex(nameIndexSecondary, newKey, secondaryKey, sizePrimaryKey, sizeSecondaryKey [i]);
                    }
                    if (changeOffset != -1){
                        file.seekp(changeOffset);
                        const string newEndList = "*-1";
                        file.write(newEndList.c_str(), sizeRecords);
                    }

                    foundEnd = true;
                }
                if (cantEncuentros == 0 && foundEnd){
                    header.refAvailList = -1;
                }
                changeOffset = lastOffset;
                cantEncuentros++;
            } while (!foundEnd);

        } else{
            // Solo hacer append
            ofstream file(fileName, ios::out | ios::binary | ios::app);
            if (!file.is_open()){
                throw runtime_error(
                        "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"File not found\" \n");
            }
            file.seekp(0, ios::end);
            int offset = file.tellp();
            file.write(fieldsString.c_str(), sizeRecords);
            // Agregar al indice
            WriteIndex(nameIndex, newKey, offset, sizePrimaryKey);
            // Agregar al archivo de indices secundarios
            for (int i = 0; i < static_cast<int>(header.secondaryKey.size()); i++){
                string nameIndexSecondary = fileName.substr(0, fileName.find(".")) + "-" + header.secondaryKey [i] + ".sdx";
                string secondaryKey = listFields [posSecondaryKey [i]];
                WriteSecondaryIndex(nameIndexSecondary, newKey, secondaryKey, sizePrimaryKey, sizeSecondaryKey [i]);
            }
            file.close();
        }
        header.cantRegistros++;
        WriteBinaryHeader(fileName, header);
        cout << "{\"result\": \x1b[38;5;46m\"OK\"\x1b[0m} \n";
    } catch (const std::exception& e){
        std::cerr << e.what() << '\n';
    }
}

void WriteFunction::Compact(const string& fileName){
    string tempName = fileName.substr(0, fileName.find("/") + 1) + "temp.bin";
    rename(fileName.c_str(), tempName.c_str());
    ifstream file(tempName, ios::in | ios::binary);
    // Crear archivo nuevo
    ofstream newFileReal(fileName, ios::binary);
    newFileReal.close();
    fstream newFile(fileName, ios::out | ios::in | ios::binary);
    if (!file.is_open() || !newFile.is_open()){
        throw runtime_error(
                "{\"result\": \x1b[38;5;197m\"ERROR\"\x1b[0m, \"error\": \"File not found\" \n");
    }
    Header header = ReadFunction::ReadBinaryHeader(tempName);
    if (header.refAvailList != -1){
        header.refAvailList = -1;
        WriteBinaryHeader(fileName, header);
        // Leer hasta encontrar el delimador del header
        bool foundEnd = false;
        file.seekg(0, ios::beg);
        while (!foundEnd){
            char Byte [1];
            file.read(Byte, 1);
            if (Byte [0] == '~'){
                foundEnd = true;
            }
        }
        int sizeRecords = 0;
        for (int i = 0; i < static_cast<int>(header.fieldsSize.size()); i++){
            sizeRecords += header.fieldsSize [i];
        }
        // ahora escribir los registros
        int contDelete = 0;
        // Elimina los archivos de indices
        string nameIndex = fileName.substr(0, fileName.find(".")) + ".idx";
        remove(nameIndex.c_str());
        ofstream fileIndex(nameIndex, ios::binary);
        fileIndex.close();
        for (int i = 0; i < static_cast<int>(header.secondaryKey.size()); i++){
            string nameIndexSecondary = fileName.substr(0, fileName.find(".")) + "-" + header.secondaryKey [i] + ".sdx";
            remove(nameIndexSecondary.c_str());
            ofstream fileIndexSecondary(nameIndexSecondary, ios::binary);
            fileIndexSecondary.close();
        }
        foundEnd = false;
        newFile.seekg(0, ios::beg);
        while (!foundEnd){
            char Byte [1];
            newFile.read(Byte, 1);
            if (Byte [0] == '~'){
                foundEnd = true;
            }
        }
        string numOffset = "";
        while (file.eof() == false){
            char buffer [sizeRecords];
            file.read(buffer, sizeRecords);
            if (buffer [0] == '*'){
                string actualOffset(buffer);
                if (numOffset != actualOffset){
                    numOffset = buffer;
                    contDelete++;
                }
            } else{
                string record(buffer);
                int offset = newFile.tellp();
                newFile.write(record.c_str(), sizeRecords);
                int sizeKey;
                // Agregar al archivo de indices secundarios
                stringstream ss(record);
                int posActual = 0;
                string field;
                string key;
                int posKey;
                int posSecondaryKeys [header.secondaryKey.size()];
                int sizeSecondaryKeys [header.secondaryKey.size()];
                for (int i = 0; i < static_cast<int>(header.fieldsName.size()); i++){
                    if (header.fieldsName [i] == header.primaryKey){
                        posKey = i;
                        sizeKey = header.fieldsSize [i];
                    }
                    for (int j = 0; j < static_cast<int>(header.secondaryKey.size()); j++){
                        if (header.fieldsName [i] == header.secondaryKey [j]){
                            posSecondaryKeys [j] = i;
                            sizeSecondaryKeys [j] = header.fieldsSize [i];
                        }
                    }
                }
                while (getline(ss, field, ',')){
                    if (posActual == posKey){
                        key = field;
                        WriteIndex(nameIndex, key, offset, sizeKey);
                    }
                    for (int i = 0; i < static_cast<int>(header.secondaryKey.size()); i++){
                        if (posActual == posSecondaryKeys [i]){
                            string nameIndexSecondary = fileName.substr(0, fileName.find(".")) + "-" + header.secondaryKey [i] + ".sdx";
                            string secondaryKey = field;
                            WriteSecondaryIndex(nameIndexSecondary, key, secondaryKey, sizeKey, sizeSecondaryKeys [i]);
                        }
                    }
                    posActual++;
                }
            }
        }
        file.close();
        remove(tempName.c_str());
        cout << "{\"result\": \x1b[38;5;46m\"OK\"\x1b[0m, \"records-reclaimed\": 	\x1b[38;5;226m" << contDelete << "\x1b[0m} \n";
    } else{
        remove(fileName.c_str());
        rename(tempName.c_str(), fileName.c_str());
        cout << "{\"result\": \x1b[38;5;46m\"OK\"\x1b[0m, \"records-reclaimed\": 0} \n";
    }
}