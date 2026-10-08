#ifndef VALIDACIONES_H
#define VALIDACIONES_H

#include <string>

#include "Orden.h"

struct ResultadoValidacion
{
    bool valido = false;
    std::string mensaje;
};

bool coordenadaValida(int x, int y);
bool posicionesAdyacentes(int x1, int y1, int x2, int y2);
ResultadoValidacion validarMemoria(const Memoria& memoria);

#endif