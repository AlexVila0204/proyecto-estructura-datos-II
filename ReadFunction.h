#ifndef READFUNCTION_H
#define READFUNCTION_H
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>
#include "nlohmann/json.hpp"
#include "WriteFunction.h"
using json = nlohmann::json;
using std::binary_search;
using std::cerr;
using std::copy;
using std::cout;
using std::endl;
using std::exception;
using std::fstream;
using std::ifstream;
using std::ios;
using std::lower_bound;
using std::ofstream;
using std::regex;
using std::regex_match;
using std::runtime_error;
using std::setw;
using std::sort;
using std::stoi;
using std::strcpy;
using std::string;
using std::stringstream;
using std::to_string;
using std::unordered_set;
using std::vector;
struct SDKIndex{
    string primaryKey;
    string secondaryKey;
};
struct Index{
    string key;
    int offset;
};
struct Header{
    vector<string> fieldsName;
    vector<int> fieldsSize;
    vector<string> fieldsType;
    vector<string> secondaryKey;
    string primaryKey;
    int refAvailList;
    int cantRegistros;
};
class ReadFunction{
public:
    /**
     * @brief Lee el archivo binario y muestra el header
     *
     * @param fileName nombre del archivo binario
     * @return Header el header del archivo binario
     */
    static Header ReadBinaryHeader(const string& fileName);
    /**
     * @brief Carga los datos de un archivo csv al binario
     *
     * @param fileName archivo binario
     * @param fileName2 archivo csv
     */
    static void CargarDatos(const string& fileName, const string& fileName2);
    /**
     * @brief Metodo que carga un vector con llaves primarias en un hash, este mismo verifica si la llave primaria ya existe
     *
     * @param primaryKeys vector que contiene las llaves primarias
     * @param primaryKey llave a verificar
     * @return true si la llave primaria no existe
     * @return false si la llave primaria ya existe
     */
    static bool UniqueIDMethod(vector<string>& primaryKeys, const string& primaryKey);
    /**
     * @brief Metodo que lee los registros de un archivo binario pero solo regresa las llaves primarias
     *
     * @param header header del archivo
     * @param sizeRecord (la longitud de cada registro)
     * @param file archivo binario a leer
     * @return vector<string> vector que contiene las llaves primarias
     */
    static vector<string> ReadBinaryRecordsPrimaryKey(const Header& header, int& sizeRecord, const string& fileName);
    /**
     * @brief Metodo que describe la estructura de un archivo binario
     *
     * @param fileName nombre del archivo binario
     */
    static void DescribeBinary(const string& fileName);
    /**
     * @brief Metodo que lista los registros de un archivo binario
     *
     * @param fileName nombre del archivo binario
     */
    static void ListBinary(const string& fileName);
    /**
     * @brief Metodo que busca por llave primaria
     *
     * @param fileName nombre del archivo binario
     * @param primaryKey nombre de la llave primaria
     * @param isNumber si la llave primaria es un numero
     * @param opcionSecreta opcion secreta :3
     */
    static void SearchPK(const string& fileName, const string& primaryKey, bool isNumber, bool opcionSecreta);
    /**
     * @brief Metodo para comparar dos llaves, ademas de quitar el offset
     *
     * @param key1 llave 1 a comparar
     * @param key2 llave 2 a comparar
     * @return true si llave 1 es mayor que la llave 2
     * @return false
     */
    static bool CompareKeys(const Index& key1, const Index& key2);
    /**
     * @brief Metodo para comparar dos llaves, ademas de quitar el offset en este caso si no es un numero
     *c
     * @param key1 llave 1 a comparar
     * @param key2 llave 2 a comparar
     * @return true si es menor
     * @return false si no es menor
     */
    static bool CompareKeysString(const Index& key1, const Index& key2);
    /**
     * @brief Metdo para buscar con llave secundaria
     *
     * @param fileName nombre del archivo binario
     * @param secondaryKey llave secundaria
     * @param valueKey valor de la llave secundaria
     */
    static void searchSDK(const string& fileName, const string& secondaryKey, const string& valueKey);
    /**
     * @brief Metodo que modifica el archivo secundario
     *
     * @param fileName nombre del archivo binario
     * @param keyMod llave a modificar
     */
    static void SearchOffset(const string& fileName, const string& primaryKey, bool isNumber, string& listFields, string& newKey);
    /**
     * @brief Funcion que lee las llaves primarias de los registros
     *
     * @param fileName nombre del archivo binario
     * @param sizeRecords tamaño de los registros
     * @param sizeKey tamaño de la llave primaria
     * @param keyTry la llave que se intenta ingresar
     * @return vector<string>
     */
    static vector<string> ReadBinaryRecordsKey(const string& fileName, int& sizeRecords, int& sizeKey);
};
#endif