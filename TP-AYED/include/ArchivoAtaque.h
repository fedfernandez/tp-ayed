#ifndef ARCHIVO_ATAQUE_H
#define ARCHIVO_ATAQUE_H

#include <string>
#include <vector>

#include "Orden.h"

// Lee un archivo CSV validado (reglas del ataque) y lo copia a memoria.
bool leerArchivo(const std::string &nombre, Memoria &memoria, std::string &mensaje);

// Lee las filas del archivo sin validar el ataque (formato y coordenadas).
bool leerRegistros(const std::string &nombre, std::vector<OrdenArchivo> &registros, std::string &mensaje);

// Combina los registros en una memoria (fusiona celdas repetidas).
bool construirMemoria(const std::vector<OrdenArchivo> &registros, Memoria &memoria, std::string &mensaje);

// Escribe la memoria completa como archivo.
bool escribirArchivo(const std::string &nombre, const Memoria &memoria, std::string &mensaje);

// Escribe la lista de registros como archivo.
bool escribirRegistros(const std::string &nombre, const std::vector<OrdenArchivo> &registros, std::string &mensaje);

#endif