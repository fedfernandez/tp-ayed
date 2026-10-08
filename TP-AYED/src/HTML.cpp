#include "HTML.h"

#include <fstream>

#include "Rutas.h"

namespace
{
// Clase segun el orden de precedencia de la consigna.
std::string claseCelda(const Orden& orden)
{
    if (orden.despegue)
        return "despegue";
    if (orden.aterrizaje)
        return "aterrizaje";
    if (orden.ataqueKamikaze)
        return "kamikaze";
    if (orden.soltarGranada1 || orden.soltarGranada2)
        return "ataque";
    if (orden.espera > 0)
        return "espera";
    if (!orden.sinSiguiente())
        return "movimiento";
    return "vacia";
}

std::string simboloCelda(const Orden& orden)
{
    if (orden.despegue)
        return "D";
    if (orden.aterrizaje)
        return "A";
    if (orden.ataqueKamikaze)
        return "K";
    if (orden.soltarGranada1)
        return "1";
    if (orden.soltarGranada2)
        return "2";
    if (orden.espera > 0)
        return "E";
    if (!orden.sinSiguiente())
        return "M";
    return "";
}
}

bool generarHTML(const std::string& nombre,
                 const Memoria& memoria,
                 std::string& mensaje)
{
    std::filesystem::path ruta;

    if (!rutaEnAtaques(nombre, ruta, mensaje))
        return false;

    std::ofstream archivo(ruta);

    if (!archivo.is_open())
    {
        mensaje = "No se pudo crear el archivo HTML.";
        return false;
    }

    archivo <<
        "<!DOCTYPE html>\n"
        "<html lang=\"es\">\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<title>Ruta de ataque del drone</title>\n"
        "<style>\n"
        "body { font-family: Arial, sans-serif; background: #eee; }\n"
        "h1 { text-align: center; }\n"
        "table { border-collapse: collapse; margin: auto; }\n"
        "td { width: 7px; height: 7px; border: 1px solid #ddd; "
        "text-align: center; font-size: 5px; }\n"
        ".vacia { background: #fff; }\n"
        ".despegue { background: #7ae07a; }\n"
        ".aterrizaje { background: #f5e56b; }\n"
        ".kamikaze { background: #f08080; }\n"
        ".ataque { background: #87ceeb; }\n"
        ".espera { background: #d3d3d3; }\n"
        ".movimiento { background: #c8b6f0; }\n"
        "p { text-align: center; }\n"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<h1>Ruta de ataque del drone</h1>\n"
        "<table>\n";

    for (int y = 0; y < FILAS; ++y)
    {
        archivo << "<tr>\n";

        for (int x = 0; x < COLUMNAS; ++x)
        {
            const Orden& orden = memoria.celda(x, y);
            const bool ocupada = orden.tieneAccion();

            archivo << "<td class=\"" << claseCelda(orden) << "\"";

            if (ocupada)
                archivo << " title=\"(" << x << "," << y << ") "
                        << orden.descripcion() << "\"";

            archivo << ">";

            if (ocupada)
                archivo << simboloCelda(orden);

            archivo << "</td>\n";
        }

        archivo << "</tr>\n";
    }

    archivo <<
        "</table>\n"
        "<p>D=despegue A=aterrizaje K=kamikaze 1/2=granadas "
        "E=espera M=movimiento (detalle completo sobre cada celda)</p>\n"
        "</body>\n"
        "</html>\n";

    archivo.close();

    if (!archivo)
    {
        mensaje = "Ocurrio un error al escribir el archivo HTML.";
        return false;
    }

    mensaje = "Archivo HTML generado correctamente.";
    return true;
}