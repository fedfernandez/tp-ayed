#ifndef UI_H
#define UI_H

#include <string>

#include "Orden.h"

void pausar();
void mostrarMenu();
void mostrarMemoria(const Memoria& memoria);
void mostrarCuadricula(const Memoria& memoria);

bool pedirLinea(const std::string& indicacion, std::string& valor);
bool pedirEntero(const std::string& indicacion, int& valor);
bool pedirOrden(OrdenArchivo& registro, std::string& mensaje);

#endif