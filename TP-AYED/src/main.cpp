#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "ArchivoAtaque.h"
#include "HTML.h"
#include "Orden.h"
#include "Rutas.h"
#include "UI.h"
#include "Validaciones.h"

namespace
{
    Memoria memoria;

    void limpiarPantalla()
    {
        #ifdef _WIN32
                std::system("cls");
        #else
                std::system("clear");
        #endif
    }

    void cargarAtaque()
    {
        std::vector<std::string> disponibles;

        listarArchivosAtaques(disponibles);

        if (disponibles.empty())
        {
            std::cout << "\n (La carpeta ataques no contiene archivos todavia.)\n";
        }
        else
        {
            std::cout << "\n Archivos disponibles en ataques:\n";

            for (std::size_t i = 0; i < disponibles.size(); ++i)
                std::cout << "   " << (i + 1) << ". " << disponibles[i]
                          << "\n";
        }

        std::string seleccion;

        if (!pedirLinea("\n Ingrese el numero del archivo (o el nombre): ", seleccion))
            return;

        std::string nombre;
        bool numeroValido = true;

        try
        {
            const std::size_t indice = std::stoul(seleccion);

            if (indice >= 1 && indice <= disponibles.size())
                nombre = disponibles[indice - 1];
            else
                numeroValido = false;
        }
        catch (const std::exception &)
        {
            numeroValido = false;
        }

        if (!numeroValido)
            nombre = seleccion;

        std::string mensaje;
        const bool resultado = leerArchivo(nombre, memoria, mensaje);

        std::cout << (resultado ? "\n OK: " : "\n ERROR: ")
                  << mensaje << "\n";
    }

    void mostrarAtaque()
    {
        mostrarCuadricula(memoria);
        mostrarMemoria(memoria);
    }

    void crearAtaque()
    {
        std::string nombre;

        if (!pedirLinea("\n Ingrese ruta y nombre del archivo nuevo (ej: nuevo.dat): ", nombre))
            return;

        Memoria construccion;
        int cantidad = 0;

        while (true)
        {
            std::string respuesta;

            if (!pedirLinea("\n Ingresar un registro? (s/n): ", respuesta))
                return;

            if (respuesta != "s" && respuesta != "S")
                break;

            OrdenArchivo registro;
            std::string mensaje;

            if (!pedirOrden(registro, mensaje))
                return;

            if (!mensaje.empty())
            {
                std::cout << "\n ERROR: " << mensaje << "\n";
                continue;
            }

            Orden orden;
            convertir(registro, orden);

            Orden &celda = construccion.celda(registro.x, registro.y);

            if (celda.tieneAccion())
                std::cout << "\n Aviso: ya existe una orden en (" << registro.x << "," << registro.y << "); se combinan las acciones.\n";

            std::string errorCombinar;

            if (!combinarOrden(celda, orden, errorCombinar))
            {
                std::cout << "\n ERROR: " << errorCombinar << "\n";
                continue;
            }

            ++cantidad;

            std::cout << " Registro " << cantidad << ": (" << registro.x << "," << registro.y << ") -> " << celda.descripcion() << "\n";
        }

        if (cantidad == 0)
        {
            std::cout << "\n No se ingreso ningun registro; el archivo no se crea.\n";
            return;
        }

        const ResultadoValidacion validacion = validarMemoria(construccion);

        if (!validacion.valido)
        {
            std::cout << "\n ERROR: el ataque no es valido: "
                      << validacion.mensaje << "\n";

            std::string respuesta;

            if (!pedirLinea(" Guardar de todos modos? (s/n): ", respuesta))
                return;

            if (respuesta != "s" && respuesta != "S")
            {
                std::cout << "\n El archivo no se creo.\n";
                return;
            }
        }

        std::string mensaje;
        const bool resultado = escribirArchivo(nombre, construccion, mensaje);

        if (resultado)
            memoria = construccion;

        std::cout << (resultado ? "\n OK: " : "\n ERROR: ")
                  << mensaje << "\n";
    }

    void corregirAtaque()
    {
        const int filasPrevias = FILAS;
        const int columnasPrevias = COLUMNAS;

        const auto restaurarGrilla = [&]()
        {
            FILAS = filasPrevias;
            COLUMNAS = columnasPrevias;
        };

        std::string nombre;

        if (!pedirLinea("\n Ingrese ruta relativa a la carpeta ataques: ", nombre))
            return;

        // Se lee primero el archivo para aplicar el tamano de grilla que
        // tenga guardado y asi mantener la compatibilidad.
        std::vector<OrdenArchivo> registros;
        std::string mensaje;

        if (!leerRegistros(nombre, registros, mensaje))
        {
            std::cout << "\n ERROR: " << mensaje << "\n";
            restaurarGrilla();
            return;
        }

        int x;
        int y;

        if (!pedirEntero("\n Ingrese X del registro: ", x))
        {
            restaurarGrilla();
            return;
        }

        if (!pedirEntero("\n Ingrese Y del registro: ", y))
        {
            restaurarGrilla();
            return;
        }

        if (!coordenadaValida(x, y))
        {
            std::cout << "\n ERROR: coordenadas fuera de la cuadricula.\n";
            restaurarGrilla();
            return;
        }

        std::size_t indice = registros.size();

        for (std::size_t i = 0; i < registros.size(); ++i)
        {
            if (registros[i].x == x && registros[i].y == y)
            {
                indice = i;
                break;
            }
        }

        if (indice == registros.size())
        {
            std::cout << "\n ERROR: no existe un registro en esas coordenadas.\n";
            restaurarGrilla();
            return;
        }

        Orden actual;
        convertir(registros[indice], actual);

        std::cout << "\n Registro actual: (" << registros[indice].x << ","
                  << registros[indice].y << ") -> "
                  << actual.descripcion() << "\n";

        OrdenArchivo nuevo;
        std::string error;

        if (!pedirOrden(nuevo, error))
        {
            restaurarGrilla();
            return;
        }

        if (!error.empty())
        {
            std::cout << "\n ERROR: " << error << "\n";
            restaurarGrilla();
            return;
        }

        registros[indice] = nuevo;

        Memoria temporal;

        if (!construirMemoria(registros, temporal, mensaje))
        {
            std::cout << "\n ERROR: " << mensaje
                      << "; el archivo no se modifica.\n";
            restaurarGrilla();
            return;
        }

        const ResultadoValidacion validacion = validarMemoria(temporal);

        if (!validacion.valido)
        {
            std::cout << "\n ERROR: el ataque resultante no es valido ("
                      << validacion.mensaje
                      << "); el archivo no se modifica.\n";
            restaurarGrilla();
            return;
        }

        std::string mensajeEscritura;

        if (escribirRegistros(nombre, registros, mensajeEscritura))
        {
            memoria = temporal;
            std::cout << "\n OK: " << mensajeEscritura << "\n";
        }
        else
        {
            restaurarGrilla();
            std::cout << "\n ERROR: " << mensajeEscritura << "\n";
        }
    }

    void corregirMemoria()
    {
        int x;
        int y;

        if (!pedirEntero("\n Ingrese X del registro: ", x))
            return;

        if (!pedirEntero("\n Ingrese Y del registro: ", y))
            return;

        if (!coordenadaValida(x, y))
        {
            std::cout << "\n ERROR: coordenadas fuera de la cuadricula.\n";
            return;
        }

        if (!memoria.celda(x, y).tieneAccion())
        {
            std::cout << "\n ERROR: no existe un registro en esas coordenadas.\n";
            return;
        }

        std::cout << "\n Registro actual: (" << x << "," << y << ") -> "
                  << memoria.celda(x, y).descripcion() << "\n";

        OrdenArchivo nuevo;
        std::string error;

        if (!pedirOrden(nuevo, error))
            return;

        if (!error.empty())
        {
            std::cout << "\n ERROR: " << error << "\n";
            return;
        }

        Orden orden;
        convertir(nuevo, orden);

        if (nuevo.x != x || nuevo.y != y)
        {
            if (memoria.celda(nuevo.x, nuevo.y).tieneAccion())
                std::cout << "\n Aviso: ya existia una orden en el nuevo destino; se reemplaza.\n";

            memoria.celda(x, y) = Orden();
            memoria.celda(nuevo.x, nuevo.y) = orden;
        }
        else
        {
            memoria.celda(x, y) = orden;
        }

        const ResultadoValidacion validacion = validarMemoria(memoria);

        std::cout << (validacion.valido
                          ? "\n OK: "
                          : "\n Ojo: la memoria no es valida: ")
                  << validacion.mensaje << "\n";
    }

    void guardarMemoria()
    {
        const ResultadoValidacion validacion = validarMemoria(memoria);

        if (!validacion.valido)
        {
            std::cout << "\n ERROR: no hay un ataque valido en memoria que guardar"
                      << " (" << validacion.mensaje << ").\n";
            return;
        }

        std::string nombre;

        if (!pedirLinea("\n Ingrese ruta y nombre del archivo nuevo: ", nombre))
            return;

        std::string mensaje;
        const bool resultado = escribirArchivo(nombre, memoria, mensaje);

        std::cout << (resultado ? "\n OK: " : "\n ERROR: ")
                  << mensaje << "\n";
    }
    void visualizarHtml()
    {
        std::string nombre;

        if (!pedirLinea("\n Ingrese nombre del archivo HTML (ej: ruta.html): ",
                        nombre))
            return;

        std::string mensaje;
        const bool resultado = generarHTML(nombre, memoria, mensaje);

        std::cout << (resultado ? "\n OK: " : "\n ERROR: ")
                  << mensaje << "\n";
    }

    void configurarGrilla()
    {
        int filas;
        int columnas;

        if (!pedirEntero("\n Cantidad de filas (alto): ", filas))
            return;

        if (!pedirEntero(" Cantidad de columnas (ancho): ", columnas))
            return;

        if (filas < 1 || filas > 1000 || columnas < 1 || columnas > 1000)
        {
            std::cout << "\n ERROR: el tamano debe estar entre 1x1 y 1000x1000.\n";
            return;
        }

        const bool cambio = (FILAS != filas || COLUMNAS != columnas);

        if (cambio)
        {
            FILAS = filas;
            COLUMNAS = columnas;
            memoria = Memoria();
        }

        std::cout << "\n OK: grilla configurada a "
                  << COLUMNAS << "x" << FILAS;

        if (cambio)
            std::cout << "\n     (la memoria fue reiniciada).";

        std::cout << "\n";
    }
}

int main()
{
    while (true)
    {
        limpiarPantalla();
        mostrarMenu();

        int opcion;

        if (!pedirEntero("Seleccione una opcion: ", opcion))
            break;

        switch (opcion)
        {
        case 1:
            cargarAtaque();
            break;
        case 2:
            mostrarAtaque();
            break;
        case 3:
            crearAtaque();
            break;
        case 4:
            corregirAtaque();
            break;
        case 5:
            corregirMemoria();
            break;
        case 6:
            guardarMemoria();
            break;
        case 7:
            visualizarHtml();
            break;
        case 8:
            configurarGrilla();
            break;
        case 0:
            std::cout << "\n Programa finalizado.\n";
            return 0;
        default:
            std::cout << "\n Opcion invalida.\n";
            break;
        }

        pausar();
    }

    return 0;
}
