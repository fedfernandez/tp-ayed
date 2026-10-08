#include "Validaciones.h"

#include <cmath>
#include <vector>

bool coordenadaValida(int x, int y)
{
    return x >= 0 && x < COLUMNAS && y >= 0 && y < FILAS;
}

bool posicionesAdyacentes(int x1, int y1, int x2, int y2)
{
    const int deltaX = std::abs(x2 - x1);
    const int deltaY = std::abs(y2 - y1);

    return deltaX <= 1 && deltaY <= 1 &&
           !(deltaX == 0 && deltaY == 0);
}

ResultadoValidacion validarMemoria(const Memoria &memoria)
{
    ResultadoValidacion resultado;

    auto error = [&resultado](const std::string &mensaje) -> ResultadoValidacion
    {
        resultado.valido = false;
        resultado.mensaje = mensaje;
        return resultado;
    };

    int cantidadRegistros = 0;
    int cantidadDespegues = 0;
    int cantidadFinales = 0;
    bool finalAterrizaje = false;
    bool finalKamikaze = false;
    int despegueX = -1;
    int despegueY = -1;
    int finalX = -1;
    int finalY = -1;

    for (int y = 0; y < FILAS; ++y)
    {
        for (int x = 0; x < COLUMNAS; ++x)
        {
            const Orden &orden = memoria.celda(x, y);

            if (!orden.tieneAccion())
                continue;

            ++cantidadRegistros;

            if (orden.despegue)
            {
                ++cantidadDespegues;
                despegueX = x;
                despegueY = y;

                if (orden.tieneAtaque())
                    return error("El despegue no puede estar acompanado por ningun tipo de ataque.");
            }

            if (orden.aterrizaje)
            {
                ++cantidadFinales;
                finalAterrizaje = true;
                finalX = x;
                finalY = y;

                if (!orden.sinSiguiente())
                    return error("La instruccion de aterrizaje no puede tener una siguiente posicion.");
            }

            if (orden.ataqueKamikaze)
            {
                ++cantidadFinales;
                finalKamikaze = true;
                finalX = x;
                finalY = y;

                if (!orden.sinSiguiente())
                    return error("La instruccion de kamikaze no puede tener una siguiente posicion.");
            }
        }
    }

    if (cantidadRegistros == 0)
        return error("El ataque no contiene ninguna orden.");

    if (cantidadDespegues != 1)
        return error("Debe existir exactamente una instruccion de despegue.");

    if (cantidadFinales != 1 || (finalAterrizaje && finalKamikaze))
        return error("Debe existir exactamente una instruccion de aterrizaje o una de kamikaze como fin de la ruta.");

    std::vector<std::vector<bool>> visitadas(
        FILAS, std::vector<bool>(COLUMNAS, false));

    int actualX = despegueX;
    int actualY = despegueY;
    int cantidadVisitadas = 0;
    bool llegoAlFinal = false;

    while (!llegoAlFinal)
    {
        if (!coordenadaValida(actualX, actualY))
            return error("La ruta accede a una coordenada fuera de la tabla.");

        if (visitadas[static_cast<std::size_t>(actualY)][static_cast<std::size_t>(actualX)])
            return error("La ruta pasa por las mismas coordenadas mas de una vez.");

        const Orden &actual = memoria.celda(actualX, actualY);

        if (!actual.tieneAccion())
            return error("La ruta contiene un hueco: una celda vacia.");

        visitadas[static_cast<std::size_t>(actualY)][static_cast<std::size_t>(actualX)] = true;
        ++cantidadVisitadas;

        if (actual.esFinal())
        {
            if (actualX != finalX || actualY != finalY)
                return error("La ruta no termina en el fin indicado.");

            llegoAlFinal = true;
            break;
        }

        if (actual.sinSiguiente())
            return error("La ruta se corta en (" + std::to_string(actualX) + "," + std::to_string(actualY) + "): falta la siguiente posicion.");

        if (!coordenadaValida(actual.siguientex, actual.siguientey))
            return error("Una orden apunta a una coordenada fuera de la tabla.");

        if (!posicionesAdyacentes(actualX, actualY, actual.siguientex, actual.siguientey))
            return error("La ruta salta entre celdas no adyacentes.");

        actualX = actual.siguientex;
        actualY = actual.siguientey;
    }

    if (cantidadVisitadas != cantidadRegistros)
        return error("La ruta no esta completa: hay registros que no forman parte del recorrido desde el despegue hasta el fin.");

    resultado.valido = true;
    resultado.mensaje = "Ataque valido.";
    return resultado;
}