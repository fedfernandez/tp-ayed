#include "UI.h"

#include <iomanip>
#include <iostream>
#include <string>

#include "Validaciones.h"

void pausar()
{
    std::cout << "\nPresione ENTER para continuar...\n";
    std::string linea;
    std::getline(std::cin, linea);
}

void mostrarMenu()
{
    std::cout << "\n========================================\n"
                 "       SISTEMA DE GUIA DE DRONES\n"
                 "========================================\n"
                 "1. Cargar archivo de ataque en memoria\n"
                 "2. Mostrar ataque cargado\n"
                 "3. Crear un archivo de ataque nuevo\n"
                 "4. Corregir un registro del archivo\n"
                 "5. Corregir un registro en memoria\n"
                 "6. Guardar memoria en un archivo nuevo\n"
                 "7. Visualizar un archivo de ataque en HTML\n"
                 "8. Configurar tamano de la grilla\n"
                 "0. Salir\n"
                 "========================================\n";
}

void mostrarMemoria(const Memoria &memoria)
{
    std::cout << "\n========================================\n"
                 "             ATAQUE CARGADO\n"
                 "========================================\n";

    int contador = 0;
    int hoja = 1;

    for (int y = 0; y < FILAS; ++y)
    {
        for (int x = 0; x < COLUMNAS; ++x)
        {
            const Orden &orden = memoria.celda(x, y);

            if (!orden.tieneAccion())
                continue;

            std::cout << "(" << x << "," << y << ") -> "
                      << orden.descripcion() << "\n";
            ++contador;

            if (contador % 20 == 0)
            {
                std::cout << "\n--- Hoja " << hoja << " ---\n";
                pausar();
                ++hoja;
            }
        }
    }

    if (contador == 0)
        std::cout << "No hay ataque cargado en memoria.\n";
    else
        std::cout << "\n--- Fin del ataque ---\n";
}

namespace
{
    // Simbolo de una celda segun la precedencia de la consigna
    // (' ' = posicion sin accion).
    char simboloCelda(const Orden &orden, int x, int y)
    {
        if (orden.despegue)
            return 'D';
        if (orden.aterrizaje)
            return 'A';
        if (orden.soltarGranada1)
            return '1';
        if (orden.soltarGranada2)
            return '2';
        if (orden.ataqueKamikaze)
            return 'K';
        if (orden.espera > 0)
            return 'E';

        if (!orden.esFinal() && !orden.sinSiguiente())
        {
            const int dx = orden.siguientex - x;
            const int dy = orden.siguientey - y;

            if (dx > 0)
                return '>';
            if (dx < 0)
                return '<';
            if (dy > 0)
                return 'v';
            if (dy < 0)
                return '^';
            return '*';
        }

        return ' ';
    }
}

void mostrarCuadricula(const Memoria &memoria)
{
    std::cout << "\n Cuadricula completa (" << COLUMNAS << "x"
              << FILAS << "):\n\n";

    std::cout << std::setw(4) << "";

    for (int x = 0; x < COLUMNAS; ++x)
        std::cout << std::setw(3) << x;

    std::cout << "\n";

    for (int y = 0; y < FILAS; ++y)
    {
        std::cout << std::setw(4) << y;

        for (int x = 0; x < COLUMNAS; ++x)
        {
            const char simbolo = simboloCelda(memoria.celda(x, y), x, y);
            std::cout << '[' << simbolo << ']';
        }

        std::cout << "\n";
    }

    std::cout << "\n Referencias:"
              << "\n [D] despegue"
              << "\n [A] aterrizaje"
              << "\n [1] granada 1"
              << "\n [2] granada 2"
              << "\n [K] kamikaze"
              << "\n [E] espera"
              << "\n [>] [<] [v] [^] movimiento"
              << "\n [ ] vacia\n";
}

bool pedirLinea(const std::string &indicacion, std::string &valor)
{
    std::cout << indicacion;

    if (!std::getline(std::cin, valor))
        return false;

    if (!valor.empty() && valor.back() == '\r')
        valor.pop_back();

    return true;
}

bool pedirEntero(const std::string &indicacion, int &valor)
{
    std::cout << indicacion;
    std::string linea;

    if (!std::getline(std::cin, linea))
        return false;

    const std::size_t inicio = linea.find_first_not_of(" \t");

    if (inicio == std::string::npos)
    {
        std::cout << "\n Entrada vacia.\n";
        return pedirEntero(indicacion, valor);
    }

    try
    {
        valor = std::stoi(linea.substr(inicio));
        return true;
    }
    catch (...)
    {
        std::cout << "\n Entrada invalida.\n";
        return pedirEntero(indicacion, valor);
    }
}

bool pedirOrden(OrdenArchivo &registro, std::string &mensaje)
{
    registro = OrdenArchivo{};
    mensaje.clear();

    std::cout << "\n --- NUEVA ORDEN ---\n";

    int x;
    int y;

    if (!pedirEntero("Posicion X: ", x))
        return false;

    if (!pedirEntero("Posicion Y: ", y))
        return false;

    if (!coordenadaValida(x, y))
    {
        mensaje = "La posicion (" + std::to_string(x) + "," + std::to_string(y) + ") esta fuera de la cuadricula " + std::to_string(COLUMNAS) + "x" + std::to_string(FILAS) + ".";
        return false;
    }

    registro.x = x;
    registro.y = y;

    std::cout << "\nAccion de la orden:\n"
                 "  1. Despegue\n"
                 "  2. Soltar granada 1\n"
                 "  3. Soltar granada 2\n"
                 "  4. Ataque kamikaze\n"
                 "  5. Aterrizaje\n"
                 "  6. Esperar\n"
                 "  7. Mover a otra posicion\n";

    int accion;

    if (!pedirEntero("Seleccione la accion: ", accion))
        return false;

    switch (accion)
    {
    case 1:
        registro.despegue = true;
        break;

    case 2:
        registro.soltarGranada1 = true;
        break;

    case 3:
        registro.soltarGranada2 = true;
        break;

    case 4:
        registro.ataqueKamikaze = true;
        break;

    case 5:
        registro.aterrizaje = true;
        break;

    case 6:
    {
        int espera;

        if (!pedirEntero("Tiempo de espera: ", espera))
            return false;

        if (espera < 0)
        {
            mensaje = "El tiempo de espera no puede ser negativo.";
            return false;
        }

        registro.espera = static_cast<unsigned int>(espera);
        break;
    }

    case 7:
        break;

    default:
        mensaje = "Accion invalida; elija entre 1 y 7.";
        return false;
    }

    // Las acciones finales (kamikaze o aterrizaje) no tienen posicion siguiente.
    if (registro.ataqueKamikaze || registro.aterrizaje)
        return true;

    const bool esMover = (accion == 7);

    int destinoX;
    int destinoY;

    if (!pedirEntero(esMover ? "Destino X: " : "Posicion siguiente X: ",
                     destinoX))
        return false;

    if (!pedirEntero(esMover ? "Destino Y: " : "Posicion siguiente Y: ",
                     destinoY))
        return false;

    if (!coordenadaValida(destinoX, destinoY))
    {
        mensaje = (esMover ? "El destino esta fuera de la cuadricula " :
                             "La posicion siguiente esta fuera de la cuadricula ") +
                  std::to_string(COLUMNAS) + "x" +
                  std::to_string(FILAS) + ".";
        return false;
    }

    if (destinoX == 0 && destinoY == 0)
    {
        mensaje = "La posicion (0,0) representa 'sin movimiento' y no "
                  "puede usarse como siguiente.";
        return false;
    }

    registro.siguientex = destinoX;
    registro.siguientey = destinoY;

    return true;
}