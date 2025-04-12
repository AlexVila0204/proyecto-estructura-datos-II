#ifndef WRITEFUNCTION_H
#define WRITEFUNCTION_H
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
#include "ReadFunction.h"
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
struct Header;
struct SDKIndex;
struct Index;
class WriteFunction{
public:
    /**
     * @brief Crea un header del archivos json, asi como crea el indice y la llaves primarias y secundarias
     *
     * @param fileName nombre del archivo json
     */
    static void CreateFile(const string& fileName);
    /**
     * @brief Archivo para recrear el header por si sufre cambios
     *
     * @param fileName nombre del archivo binario
     * @param header header del archivo binario
     */
    static void WriteBinaryHeader(const string& fileName, Header& header);
    /**
     * @brief Metodo que carga un vector con los registros del archivo csv y otro vector con el tamaño de los registros
     *
     * @param fileName nombre del archivo binario
     * @param records vector que contiene los registros
     * @param sizeRecords vector que contiene el tamaño de los registros
     * @param primaryKeys Llave primaria
     * @param header header del archivo binario
     */
    static void WriteBinaryHeaderRecords(const string& fileName, vector<string>& records, vector<int>& sizeRecords, string& primaryKey, int& sizePrimaryKey, Header& header);
    /**
     * @brief Metodo que trae un llave primaria y un offset para escribirlo.
     *
     * @param fileName nombre del archivo binario
     * @param key llave primaria
     * @param offset lugar donde se escribira la llave primaria
     * @param sizeKey tamaño de la llave primaria
     */
    static void WriteIndex(const string& fileName, string& key, int& offset, int& sizeKey);
    /**
     * @brief Metoddo que escribe el archivo de indice secundario
     *
     * @param fileName nombre del archivo binario
     * @param refPrimaryKey referencia a la llave primaria
     * @param secondaryKey llave secundaria
     * @param sizeKey tamaño de la llave primaria
     * @param sizeSecondaryKey tamaño de la llave secundaria
     */
    static void WriteSecondaryIndex(const string& fileName, string& refPrimaryKey, const string& secondaryKey, int& sizeKey, int& sizeSecondaryKey);
    /**
     * @brief Metodo para eliminar un registro
     *
     * @param fileName nombre del archivo binario
     * @param primaryKey llave primaria
     * @param isNumber si la llave primaria es un numero
     */
    static void DeleteRecord(const string& fileName, const string& primaryKey, bool isNumber);
    /**
     * @brief Metodo que modifica un registro
     *
     * @param fileName nombre del archivo binario
     * @param primaryKey la llave primaria
     * @param isNumber determina si la llave primaria es un numero
     * @param data datos a modificar
     */
    static void ModRecord(const string& fileName, const string& primaryKey, bool isNumber, const string& data);
    /**
     * @brief Metodo que busca el offset de una llave primaria
     *
     * @param fileName  nombre del archivo binario
     * @param primaryKey llave primaria
     * @param isNumber si la llave primaria es un numero
     * @return int retorna el offset
     */
    static void ModSdx(const string& fileName, const string& keyMod, int& sizeSecondaryKey, const string& searchKey, string& newSecondaryKey);
    /**
     * @brief Metodo que agrega un registro
     *
     * @param fileName nombre del archivo binario
     * @param data datos de los campos
     */
    static void AddRecord(const string& fileName, string data);
    /**
     * @brief Metodo que compacta el archivo binario
     *
     * @param fileName nombre del archivo binario
     */
    static void Compact(const string& fileName);
};
#endif