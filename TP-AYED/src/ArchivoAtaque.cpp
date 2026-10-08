#include "ArchivoAtaque.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

#include "Rutas.h"
#include "Validaciones.h"

namespace
{
    constexpr const char *ENCABEZADO = "x,y,espera,bomba1,bomba2,kamikaze,aterrizaje,despegue,siguiente_x,siguiente_y";

    // JSON helpers
    std::string trimJson(const std::string &s)
    {
        const std::size_t inicio = s.find_first_not_of(" \t\r\n");
        if (inicio == std::string::npos)
            return "";
        const std::size_t fin = s.find_last_not_of(" \t\r\n");
        return s.substr(inicio, fin - inicio + 1);
    }

    bool extraerValorJson(const std::string &json, const std::string &clave, std::string &valor)
    {
        const std::string buscador = "\"" + clave + "\"";
        std::size_t pos = json.find(buscador);
        if (pos == std::string::npos)
            return false;
        pos = json.find(':', pos + buscador.size());
        if (pos == std::string::npos)
            return false;
        const std::size_t inicio = json.find_first_not_of(" \t\r\n", pos + 1);
        if (inicio == std::string::npos)
            return false;
        const char c = json[inicio];
        if (c == '"')
        {
            const std::size_t finStr = json.find('"', inicio + 1);
            if (finStr == std::string::npos)
                return false;
            valor = json.substr(inicio + 1, finStr - (inicio + 1));
            return true;
        }
        else if (c == '{' || c == '[')
        {
            const char abierto = c;
            const char cerrado = (c == '{') ? '}' : ']';
            int profundidad = 1;
            std::size_t i = inicio + 1;
            bool escape = false;
            for (; i < json.size() && profundidad > 0; ++i)
            {
                if (escape)
                {
                    escape = false;
                    continue;
                }
                if (json[i] == '\\')
                {
                    escape = true;
                    continue;
                }
                if (json[i] == abierto)
                    ++profundidad;
                else if (json[i] == cerrado)
                    --profundidad;
            }
            if (profundidad != 0)
                return false;
            valor = json.substr(inicio, i - inicio);
            return true;
        }
        else
        {
            std::size_t finVal = json.find_first_of(",}\r\n]", inicio);
            if (finVal == std::string::npos)
                finVal = json.size();
            valor = trimJson(json.substr(inicio, finVal - inicio));
            return !valor.empty();
        }
    }

    bool extraerArrayJson(const std::string &json, const std::string &clave, std::string &arrayContenido)
    {
        const std::string buscador = "\"" + clave + "\"";
        std::size_t pos = json.find(buscador);
        if (pos == std::string::npos)
            return false;
        pos = json.find(':', pos + buscador.size());
        if (pos == std::string::npos)
            return false;
        const std::size_t inicio = json.find_first_not_of(" \t\r\n", pos + 1);
        if (inicio == std::string::npos || json[inicio] != '[')
            return false;
        int profundidad = 1;
        std::size_t i = inicio + 1;
        bool escape = false;
        for (; i < json.size() && profundidad > 0; ++i)
        {
            if (escape)
            {
                escape = false;
                continue;
            }
            if (json[i] == '\\')
            {
                escape = true;
                continue;
            }
            if (json[i] == '[')
                ++profundidad;
            else if (json[i] == ']')
                --profundidad;
        }
        if (profundidad != 0)
            return false;
        arrayContenido = json.substr(inicio + 1, (i - 1) - (inicio + 1));
        return true;
    }

    bool leerEnteroCsv(std::istringstream &linea, int &valor)
    {
        std::string campo;

        if (!std::getline(linea, campo, ','))
            return false;

        std::size_t cantidadCaracteres = 0;

        try
        {
            valor = std::stoi(campo, &cantidadCaracteres);
        }
        catch (...)
        {
            return false;
        }

        return cantidadCaracteres == campo.size();
    }

    bool leerRegistroCsv(const std::string &texto, OrdenArchivo &registro)
    {
        std::string linea = texto;

        if (!linea.empty() && linea.back() == '\r')
            linea.pop_back();

        std::istringstream entrada(linea);

        int espera;
        int bomba1;
        int bomba2;
        int kamikaze;
        int aterrizaje;
        int despegue;

        if (!leerEnteroCsv(entrada, registro.x) ||
            !leerEnteroCsv(entrada, registro.y) ||
            !leerEnteroCsv(entrada, espera) ||
            !leerEnteroCsv(entrada, bomba1) ||
            !leerEnteroCsv(entrada, bomba2) ||
            !leerEnteroCsv(entrada, kamikaze) ||
            !leerEnteroCsv(entrada, aterrizaje) ||
            !leerEnteroCsv(entrada, despegue) ||
            !leerEnteroCsv(entrada, registro.siguientex) ||
            !leerEnteroCsv(entrada, registro.siguientey))
            return false;

        std::string sobrante;

        if (std::getline(entrada, sobrante, ','))
            return false;

        if (espera < 0 ||
            bomba1 < 0 || bomba1 > 1 ||
            bomba2 < 0 || bomba2 > 1 ||
            kamikaze < 0 || kamikaze > 1 ||
            aterrizaje < 0 || aterrizaje > 1 ||
            despegue < 0 || despegue > 1)
            return false;

        registro.espera = static_cast<unsigned int>(espera);
        registro.soltarGranada1 = bomba1 != 0;
        registro.soltarGranada2 = bomba2 != 0;
        registro.ataqueKamikaze = kamikaze != 0;
        registro.aterrizaje = aterrizaje != 0;
        registro.despegue = despegue != 0;

        return true;
    }

    bool leerFilas(const std::string &nombre, std::vector<OrdenArchivo> &registros, std::string &mensaje)
    {
        std::filesystem::path ruta;

        if (!rutaEnAtaques(nombre, ruta, mensaje))
            return false;

        std::ifstream archivo(ruta);

        if (!archivo.is_open())
        {
            mensaje = "No se pudo abrir el archivo dentro de la carpeta ataques.";
            return false;
        }

        std::string primeraLinea;

        if (!std::getline(archivo, primeraLinea))
        {
            mensaje = "El archivo esta vacio.";
            return false;
        }

        if (!primeraLinea.empty() && primeraLinea.back() == '\r')
            primeraLinea.pop_back();

        if (primeraLinea != ENCABEZADO)
        {
            mensaje = "El archivo no tiene el formato CSV del trabajo practico.";
            return false;
        }

        registros.clear();

        int cantidadRegistros = 0;
        std::string linea;

        while (std::getline(archivo, linea))
        {
            if (linea.empty())
                continue;

            ++cantidadRegistros;

            OrdenArchivo registro;

            if (!leerRegistroCsv(linea, registro))
            {
                mensaje = "Formato invalido en el registro " + std::to_string(cantidadRegistros) + ".";
                return false;
            }

            if (!coordenadaValida(registro.x, registro.y))
            {
                mensaje = "Coordenada invalida en el registro " + std::to_string(cantidadRegistros) + ".";
                return false;
            }

            if (!registro.aterrizaje && !registro.ataqueKamikaze &&
                !coordenadaValida(registro.siguientex, registro.siguientey))
            {
                mensaje = "Siguiente posicion invalida en el registro " + std::to_string(cantidadRegistros) + ".";
                return false;
            }

            registros.push_back(registro);
        }

        if (archivo.bad())
        {
            mensaje = "Ocurrio un error al leer el archivo.";
            return false;
        }

        if (registros.empty())
        {
            mensaje = "El archivo no contiene registros.";
            return false;
        }

        return true;
    }

    bool esNumeroJson(const std::string &s)
    {
        std::string t = trimJson(s);
        if (t.empty())
            return false;
        std::size_t i = 0;
        if (t[0] == '-' || t[0] == '+')
            ++i;
        if (i == t.size())
            return false;
        bool tiene = false;
        for (; i < t.size(); ++i)
        {
            if (std::isdigit(static_cast<unsigned char>(t[i])))
                tiene = true;
            else
                return false;
        }
        return tiene;
    }

    bool leerRegistroJsonObj(const std::string &obj, OrdenArchivo &registro)
    {
        std::string v;
        if (!extraerValorJson(obj, "x", v) || !esNumeroJson(v))
            return false;
        registro.x = std::stoi(v);
        if (!extraerValorJson(obj, "y", v) || !esNumeroJson(v))
            return false;
        registro.y = std::stoi(v);
        if (extraerValorJson(obj, "espera", v) && esNumeroJson(v))
            registro.espera = static_cast<unsigned int>(std::stoi(v));
        else
            registro.espera = 0;
        if (extraerValorJson(obj, "bomba1", v))
            registro.soltarGranada1 = (v == "true" || v == "1");
        if (extraerValorJson(obj, "bomba2", v))
            registro.soltarGranada2 = (v == "true" || v == "1");
        if (extraerValorJson(obj, "granada1", v) && !extraerValorJson(obj, "bomba1", v)) // alias opcional
            registro.soltarGranada1 = (v == "true" || v == "1");
        if (extraerValorJson(obj, "granada2", v) && !extraerValorJson(obj, "bomba2", v))
            registro.soltarGranada2 = (v == "true" || v == "1");
        if (extraerValorJson(obj, "kamikaze", v))
            registro.ataqueKamikaze = (v == "true" || v == "1");
        if (extraerValorJson(obj, "aterrizaje", v))
            registro.aterrizaje = (v == "true" || v == "1");
        if (extraerValorJson(obj, "despegue", v))
            registro.despegue = (v == "true" || v == "1");
        if (extraerValorJson(obj, "siguiente_x", v) && esNumeroJson(v))
            registro.siguientex = std::stoi(v);
        else if (extraerValorJson(obj, "siguientex", v) && esNumeroJson(v))
            registro.siguientex = std::stoi(v);
        if (extraerValorJson(obj, "siguiente_y", v) && esNumeroJson(v))
            registro.siguientey = std::stoi(v);
        else if (extraerValorJson(obj, "siguientey", v) && esNumeroJson(v))
            registro.siguientey = std::stoi(v);
        return true;
    }

    bool leerFilasJson(const std::string &nombre, std::vector<OrdenArchivo> &registros, int &filas, int &columnas, bool &tieneTamano, std::string &mensaje)
    {
        std::filesystem::path ruta;
        if (!rutaEnAtaques(nombre, ruta, mensaje))
            return false;
        std::ifstream archivo(ruta);
        if (!archivo.is_open())
        {
            mensaje = "No se pudo abrir el archivo dentro de la carpeta ataques.";
            return false;
        }
        std::string contenido((std::istreambuf_iterator<char>(archivo)), std::istreambuf_iterator<char>());
        archivo.close();
        if (contenido.find("registros") == std::string::npos && contenido.find("ataque") == std::string::npos && contenido.find("\"x\"") == std::string::npos)
        {
            // no es JSON claro
            mensaje = "Formato JSON no reconocido.";
            return false;
        }
        std::string v;
        tieneTamano = false;
        if (extraerValorJson(contenido, "filas", v) && esNumeroJson(v))
        {
            filas = std::stoi(v);
            tieneTamano = true;
        }
        if (extraerValorJson(contenido, "columnas", v) && esNumeroJson(v))
        {
            columnas = std::stoi(v);
            tieneTamano = true;
        }
        if (extraerValorJson(contenido, "alto", v) && esNumeroJson(v) && !tieneTamano)
        {
            filas = std::stoi(v);
            tieneTamano = true;
        }
        if (extraerValorJson(contenido, "ancho", v) && esNumeroJson(v) && !tieneTamano)
        {
            columnas = std::stoi(v);
            tieneTamano = true;
        }
        if (extraerValorJson(contenido, "tamano", v) || extraerValorJson(contenido, "grid", v))
        {
            // intentar extraer filas/columnas anidados simples
            std::string t = v;
            if (!t.empty() && t[0] == '{')
            {
                std::string vf, vc;
                if (extraerValorJson(t, "filas", vf) && esNumeroJson(vf))
                {
                    filas = std::stoi(vf);
                    tieneTamano = true;
                }
                if (extraerValorJson(t, "columnas", vc) && esNumeroJson(vc))
                {
                    columnas = std::stoi(vc);
                    tieneTamano = true;
                }
            }
        }
        // Si el archivo guarda el tamano de la cuadricula, lo aplicamos antes
        // de validar las coordenadas para mantener la compatibilidad.
        if (tieneTamano && filas >= 1 && filas <= 1000 && columnas >= 1 && columnas <= 1000)
        {
            FILAS = filas;
            COLUMNAS = columnas;
        }

        std::string array;
        if (!extraerArrayJson(contenido, "registros", array) && !extraerArrayJson(contenido, "ataque", array))
        {
            mensaje = "No se encontraron registros en el archivo JSON.";
            return false;
        }
        registros.clear();
        std::size_t i = 0;
        int cantidad = 0;
        while (i <= array.size())
        {
            if (i == array.size() || array[i] == '}')
            {
                if (i > 0)
                {
                    std::size_t inicioObj = array.rfind('{', i - 1);
                    if (inicioObj != std::string::npos)
                    {
                        std::string obj = array.substr(inicioObj, i - inicioObj + 1);
                        OrdenArchivo reg;
                        if (leerRegistroJsonObj(obj, reg))
                        {
                            if (!coordenadaValida(reg.x, reg.y))
                            {
                                mensaje = "Coordenada invalida en el registro " + std::to_string(cantidad + 1) + ".";
                                return false;
                            }
                            if (!reg.aterrizaje && !reg.ataqueKamikaze && !coordenadaValida(reg.siguientex, reg.siguientey))
                            {
                                mensaje = "Siguiente posicion invalida en el registro " + std::to_string(cantidad + 1) + ".";
                                return false;
                            }
                            registros.push_back(reg);
                            ++cantidad;
                        }
                    }
                }
                if (i == array.size())
                    break;
                ++i;
                continue;
            }
            ++i;
        }
        if (registros.empty())
        {
            mensaje = "El archivo JSON no contiene registros validos.";
            return false;
        }
        return true;
    }
}

bool construirMemoria(const std::vector<OrdenArchivo> &registros, Memoria &memoria, std::string &mensaje)
{
    Memoria temporal;

    for (std::size_t i = 0; i < registros.size(); ++i)
    {
        Orden orden;
        convertir(registros[i], orden);

        std::string errorCombinar;

        if (!combinarOrden(temporal.celda(registros[i].x, registros[i].y), orden, errorCombinar))
        {
            mensaje = "Registro " + std::to_string(i + 1) + ": " + errorCombinar;
            return false;
        }
    }

    memoria = temporal;
    return true;
}

bool leerArchivo(const std::string &nombre, Memoria &memoria, std::string &mensaje)
{
    const int filasPrevias = FILAS;
    const int columnasPrevias = COLUMNAS;

    std::vector<OrdenArchivo> registros;
    std::string mensajeInterno;

    // Intentar leer como JSON primero (por contenido o extension)
    bool esJson = false;
    {
        std::filesystem::path ruta;
        if (rutaEnAtaques(nombre, ruta, mensajeInterno))
        {
            if (ruta.extension() == ".json")
                esJson = true;
            else
            {
                // inspeccionar contenido
                std::ifstream f(ruta);
                if (f.is_open())
                {
                    std::string inicio;
                    std::getline(f, inicio);
                    inicio = trimJson(inicio);
                    if (!inicio.empty() && inicio[0] == '{')
                        esJson = true;
                    f.close();
                }
            }
        }
    }

    bool leido = false;
    int filasCfg = FILAS;
    int colsCfg = COLUMNAS;
    bool tieneTamano = false;
    if (esJson)
    {
        if (leerFilasJson(nombre, registros, filasCfg, colsCfg, tieneTamano, mensajeInterno))
            leido = true;
        else if (leerFilas(nombre, registros, mensajeInterno))
            leido = true;
    }
    else
    {
        if (leerFilas(nombre, registros, mensajeInterno))
            leido = true;
        else if (leerFilasJson(nombre, registros, filasCfg, colsCfg, tieneTamano, mensajeInterno))
            leido = true;
    }

    if (!leido)
    {
        FILAS = filasPrevias;
        COLUMNAS = columnasPrevias;
        mensaje = mensajeInterno;
        if (mensaje.empty())
            mensaje = "No se pudo leer el archivo.";
        return false;
    }

    Memoria temporal;

    if (!construirMemoria(registros, temporal, mensaje))
    {
        FILAS = filasPrevias;
        COLUMNAS = columnasPrevias;
        return false;
    }

    const ResultadoValidacion validacion = validarMemoria(temporal);

    if (!validacion.valido)
    {
        FILAS = filasPrevias;
        COLUMNAS = columnasPrevias;
        mensaje = "El archivo se leyo, pero el ataque no es valido: " + validacion.mensaje;
        return false;
    }

    memoria = temporal;
    mensaje = "Archivo cargado correctamente.";
    return true;
}

bool leerRegistros(const std::string &nombre, std::vector<OrdenArchivo> &registros, std::string &mensaje)
{
    const int filasPrevias = FILAS;
    const int columnasPrevias = COLUMNAS;

    std::string interno;
    int f = FILAS, c = COLUMNAS;
    bool t = false;
    bool intentoJson = false;

    std::filesystem::path ruta;

    if (rutaEnAtaques(nombre, ruta, interno))
    {
        if (ruta.extension() == ".json")
        {
            intentoJson = true;

            if (leerFilasJson(nombre, registros, f, c, t, interno))
                return true;
        }
        else
        {
            std::ifstream fl(ruta);

            if (fl.is_open())
            {
                std::string ini;
                std::getline(fl, ini);
                ini = trimJson(ini);
                fl.close();

                if (!ini.empty() && ini[0] == '{')
                {
                    intentoJson = true;

                    if (leerFilasJson(nombre, registros, f, c, t, interno))
                        return true;
                }
            }
        }
    }

    std::string mensajeCsv;

    if (leerFilas(nombre, registros, mensajeCsv))
        return true;

    FILAS = filasPrevias;
    COLUMNAS = columnasPrevias;

    mensaje = (intentoJson && !interno.empty()) ? interno : mensajeCsv;
    return false;
}

bool escribirArchivo(const std::string &nombre, const Memoria &memoria, std::string &mensaje)
{
    std::vector<OrdenArchivo> registros;

    for (int y = 0; y < FILAS; ++y)
    {
        for (int x = 0; x < COLUMNAS; ++x)
        {
            const Orden &orden = memoria.celda(x, y);

            if (!orden.tieneAccion())
                continue;

            OrdenArchivo registro;
            convertir(x, y, orden, registro);
            registros.push_back(registro);
        }
    }

    return escribirRegistros(nombre, registros, mensaje);
}

bool escribirRegistros(const std::string &nombre, const std::vector<OrdenArchivo> &registros, std::string &mensaje)
{
    std::filesystem::path ruta;

    if (!rutaEnAtaques(nombre, ruta, mensaje))
        return false;

    // El formato de guardado es JSON. Si se indica una extension antigua
    // (.dat/.txt) o ninguna, se guarda como .json para completar la conversion.
    std::filesystem::path rutaOut = ruta;

    if (rutaOut.extension() == ".dat" || rutaOut.extension() == ".txt" ||
        rutaOut.extension().empty())
    {
        rutaOut.replace_extension(".json");
    }

    std::ofstream archivo(rutaOut);

    if (!archivo.is_open())
    {
        mensaje = "No se pudo crear el archivo.";
        return false;
    }

    archivo << "{\n";
    archivo << "  \"filas\": " << FILAS << ",\n";
    archivo << "  \"columnas\": " << COLUMNAS << ",\n";
    archivo << "  \"tamano\": { \"filas\": " << FILAS << ", \"columnas\": " << COLUMNAS << " },\n";
    archivo << "  \"registros\": [\n";

    for (std::size_t i = 0; i < registros.size(); ++i)
    {
        const OrdenArchivo &r = registros[i];
        archivo << "    {\n";
        archivo << "      \"x\": " << r.x << ",\n";
        archivo << "      \"y\": " << r.y << ",\n";
        archivo << "      \"espera\": " << r.espera << ",\n";
        archivo << "      \"bomba1\": " << (r.soltarGranada1 ? "true" : "false") << ",\n";
        archivo << "      \"bomba2\": " << (r.soltarGranada2 ? "true" : "false") << ",\n";
        archivo << "      \"kamikaze\": " << (r.ataqueKamikaze ? "true" : "false") << ",\n";
        archivo << "      \"aterrizaje\": " << (r.aterrizaje ? "true" : "false") << ",\n";
        archivo << "      \"despegue\": " << (r.despegue ? "true" : "false") << ",\n";
        archivo << "      \"siguiente_x\": " << r.siguientex << ",\n";
        archivo << "      \"siguiente_y\": " << r.siguientey << "\n";
        archivo << "    }";
        if (i < registros.size() - 1)
            archivo << ",";
        archivo << "\n";
    }

    archivo << "  ]\n";
    archivo << "}\n";

    if (!archivo)
    {
        mensaje = "Ocurrio un error al escribir el archivo.";
        return false;
    }

    archivo.close();
    if (!archivo)
    {
        mensaje = "Ocurrio un error al escribir el archivo.";
        return false;
    }

    mensaje = "Archivo guardado correctamente.";
    return true;
}