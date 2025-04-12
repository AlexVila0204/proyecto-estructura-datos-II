// PruebaProyectoNOSE.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
/**
 * @file main.cpp
 * @author Alberth Alexander Godoy Avila 12111345
 * @brief
 * @version 0.1
 * @date 2023-06-29
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "ReadFunction.h"
#include "WriteFunction.h"
#include "nlohmann/json.hpp"
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

/**
 * @brief Estructura que contiene los campos del archivo json
 *
 */

int main(int argc, char* argv[]) {
    // Manejar excepciones
    try {
        if (argc == 3) {
            const string arg1 = argv[1];
            const string arg2 = argv[2];
            const regex rgxJson(R"(.+\.json)");
            if (arg1 == "-create" && regex_match(arg2, rgxJson)) {
                WriteFunction::CreateFile(arg2);
            } else {
                throw runtime_error(
                        "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                        " probar con -create [archivo json]\n\x1b[0m");
            }
        } else if (argc == 4) {
            const string arg1 = argv[1];
            const string arg2 = argv[2];
            const string arg3 = argv[3];
            const regex rgxValue(R"(-value=\d+)");
            const regex rgxPK(R"(-pk=\d+)");
            const regex rgxSK(R"(-sk=.+)");
            const regex rgxBinary(R"(.+\.bin)");
            if (arg1 == "-file" && arg3 == "-GET") {
                if (regex_match(arg2, rgxBinary)) {
                    ReadFunction::ListBinary(arg2);
                } else {
                    throw runtime_error(
                            "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                            " probar con -GET, -pk y -value[valorNumerico]\n\x1b[0m");
                }
            } else if (arg1 == "-file" && arg3 == "-describe") {
                if (regex_match(arg2, rgxBinary)) {
                    ReadFunction::DescribeBinary(arg2);
                } else {
                    throw runtime_error(
                            "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                            " probar con -file [archivo bin] -describe\n\x1b[0m");
                }
            } else if (arg1 == "-file" && arg3 == "-compact") {
                if (regex_match(arg2, rgxBinary)) {
                    WriteFunction::Compact(arg2);
                } else {
                    throw runtime_error(
                            "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                            " probar con -file [archivo bin] -compact\n\x1b[0m");
                }

            } else {
                throw runtime_error(
                        "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                        " probar con -GET(Para Buscar PK), GET (para buscar SK) o -file [archivo bin] -describe \n\x1b[0m");
            }
        } else if (argc == 5) {
            // Cargar Datos
            const string arg1 = argv[1];
            const string arg2 = argv[2];
            const string arg3 = argv[3];
            const string arg4 = argv[4];
            const regex rgxBin(R"(.+\.bin)");
            const regex rgxCsv(R"(.+\.csv)");
            const regex rgxPk("(^-pk=.+$)");
            regex rgxNumber("(^[0-9]+$)|(^[0-9]+\\.[0-9]+$)");
            if (arg1 == "-file" && arg3 == "-load") {
                if (regex_match(arg2, rgxBin) && regex_match(arg4, rgxCsv)) {
                    ReadFunction::CargarDatos(arg2, arg4);

                } else {
                    throw runtime_error(
                            "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                            " probar con -file, el nombre del archivo .bin , -load y el nombre del archivo .csv\n\x1b[0m");
                }
            } else if (arg1 == "-file" && arg3 == "-DELETE") {
                if (regex_match(arg2, rgxBin) && regex_match(arg4, rgxPk)) {
                    string value = arg4.substr(arg4.find("=") + 1);
                    if (regex_match(value, rgxNumber)) {
                        WriteFunction::DeleteRecord(arg2, value, true);
                    } else {
                        WriteFunction::DeleteRecord(arg2, value, false);
                    }
                } else {
                    throw runtime_error(
                            "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                            " probar con -file, el nombre del archivo .bin , -load y el nombre del archivo .csv\n\x1b[0m");
                }
            } else {
                throw runtime_error(
                        "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                        " probar con -file, el nombre del archivo .bin , -load y el nombre del archivo .csv\n\x1b[0m");
            }
            // Buscar por llave primaria
        } else if (argc == 6) {
            const string arg1 = argv[1];
            const string arg2 = argv[2];
            const string arg3 = argv[3];
            const string arg4 = argv[4];
            const string arg5 = argv[5];
            regex rgxBin(R"(.+\.bin)");
            regex rgxVal("(^-value=.+$)");
            regex rgxValSK("(^-sk=.+$)");
            regex rgxNumber("(^[0-9]+$)|(^[0-9]+\\.[0-9]+$)");
            if (arg1 == "-file" && arg3 == "-GET" && arg4 == "-pk") {
                if (regex_match(arg2, rgxBin) && regex_match(arg5, rgxVal)) {
                    string value = arg5.substr(arg5.find("=") + 1);
                    if (regex_match(value, rgxNumber)) {
                        ReadFunction::SearchPK(arg2, value, true, false);
                    } else {
                        ReadFunction::SearchPK(arg2, value, false, false);
                    }

                } else {
                    throw runtime_error(
                            "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                            " probar con -file, el nombre del archivo .bin , -GET, -pk y el valor de la llave primaria\n\x1b[0m");
                }
            } else if (arg1 == "-file" && arg3 == "-GET") {
                if (regex_match(arg2, rgxBin) && regex_match(arg5, rgxVal) && regex_match(arg4, rgxValSK)) {
                    string value = arg5.substr(arg5.find("=") + 1);
                    string sk = arg4.substr(arg4.find("=") + 1);
                    ReadFunction::searchSDK(arg2, sk, value);
                } else {
                    throw runtime_error(
                            "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                            " probar con -file, el nombre del archivo .bin , -GET, -pk y el valor de la llave primaria\n\x1b[0m");
                }
            }
        } else {
            const string arg1 = argv[1];
            const string arg2 = argv[2];
            const string arg3 = argv[3];
            const string arg4 = argv[4];
            regex rgxVal("(^-value=.+$)");
            regex rgxPk("(^-pk=.+$)");
            regex rgxBin(R"(.+\.bin)");
            regex rgxData("(^-data=.+$)");
            regex rgxNumber("(^[0-9]+$)|(^[0-9]+\\.[0-9]+$)");
            regex rgxValSK("(^-sk=.+$)");
            if (arg3 == "-PUT") {
                if (regex_match(arg2, rgxBin) && regex_match(arg4, rgxPk)) {
                    string value = arg4.substr(arg4.find("=") + 1);
                    string argData;
                    for (int i = 5; i < argc; i++) {
                        argData += argv[i];
                    }
                    if (regex_match(argData, rgxData)) {
                        string data = argData.substr(argData.find("=") + 1);
                        if (data[0] == '{' && data[data.length() - 1] == '}') {
                            data = data.substr(1, data.length() - 2);
                            if (regex_match(value, rgxNumber)) {
                                WriteFunction::ModRecord(arg2, value, true, data);
                            } else {
                                WriteFunction::ModRecord(arg2, value, false, data);
                            }
                        } else {
                            throw runtime_error(
                                    "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mFile -data= structure is Invalid\n\x1b[0m");
                        }
                    } else {
                        throw runtime_error(
                                "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mFile -data= structure is Invalid\n\x1b[0m");
                    }
                } else {
                    throw runtime_error(
                            "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                            " probar con -file, el nombre del archivo .bin , -PUT, -pk=[valordelallave] y los datos a agregar\n\x1b[0m");
                }
            } else if (arg1 == "-file" && arg3 == "-GET") {
                const string arg5 = argv[5];
                if (regex_match(arg2, rgxBin) && regex_match(arg4, rgxValSK) && regex_match(arg5, rgxVal)) {
                    string sk = arg4.substr(arg4.find("=") + 1);
                    string argData;
                    for (int i = 5; i < argc; i++) {
                        argData += argv[i];
                        argData += " ";
                    }
                    string value = argData.substr(argData.find("=") + 1);
                    value = value.substr(0, value.size() - 1);
                    ReadFunction::searchSDK(arg2, sk, value);
                } else {
                    throw runtime_error(
                            "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                            " probar con -file, el nombre del archivo .bin , -GET, -pk y el valor de la llave primaria\n\x1b[0m");
                }
            } else if (arg1 == "-file" && arg3 == "-POST") {
                if (regex_match(arg2, rgxBin)) {
                    string argData;
                    for (int i = 4; i < argc; i++) {
                        argData += argv[i];
                    }
                    if (regex_match(argData, rgxData)) {
                        string data = argData.substr(argData.find("=") + 1);
                        if (data[0] == '{' && data[data.length() - 1] == '}') {
                            data = data.substr(1, data.length() - 2);
                            WriteFunction::AddRecord(arg2, data);
                        } else {
                            throw runtime_error(
                                    "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mFile -data= structure is Invalid\n\x1b[0m");
                        }
                    } else {
                        throw runtime_error(
                                "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mFile -data= structure is Invalid\n\x1b[0m");
                    }
                } else {
                    throw runtime_error(
                            "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                            " probar con -file, el nombre del archivo .bin , -POST y los Datos\n\x1b[0m");
                }
            } else {
                throw runtime_error(
                        "\x1b[38;5;63m[ERROR] \x1b[38;5;226mERROR ARGUMENTOS: \x1b[38;5;197mEl argumento ingresado no es valido"
                        "\n\x1b[0m");
            }
        }

    } catch (const exception& e) {
        cerr << e.what() << '\n';
    }

    return 0;
}
