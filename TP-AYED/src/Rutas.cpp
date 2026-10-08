#include "Rutas.h"

namespace
{
    std::filesystem::path directorioAtaques()
    {
        std::filesystem::path actual = std::filesystem::current_path();

        while (!actual.empty())
        {
            const std::filesystem::path candidato = actual / "ataques";

            if (std::filesystem::is_directory(candidato))
                return candidato.lexically_normal();

            const std::filesystem::path padre = actual.parent_path();

            if (padre == actual)
                break;

            actual = padre;
        }

        return (std::filesystem::current_path() / "ataques").lexically_normal();
    }
}

bool rutaEnAtaques(const std::string &nombre, std::filesystem::path &ruta, std::string &mensaje)
{
    const std::filesystem::path base = directorioAtaques();
    const std::filesystem::path entrada = nombre;

    if (nombre.empty() || entrada.is_absolute())
    {
        ruta.clear();
        mensaje = "La ruta debe ser relativa a la carpeta ataques.";
        return false;
    }

    ruta = (base / entrada).lexically_normal();

    auto pasoBase = base.begin();
    auto pasoRuta = ruta.begin();

    for (; pasoBase != base.end(); ++pasoBase, ++pasoRuta)
    {
        if (pasoRuta == ruta.end() || *pasoRuta != *pasoBase)
        {
            ruta.clear();
            mensaje = "La ruta debe permanecer dentro de la carpeta ataques.";
            return false;
        }
    }

    if (ruta == base)
    {
        ruta.clear();
        mensaje = "La ruta debe indicar un archivo (no la carpeta ataques).";
        return false;
    }

    mensaje.clear();
    return true;
}

bool listarArchivosAtaques(std::vector<std::string> &nombres)
{
    nombres.clear();

    const std::filesystem::path directorio = directorioAtaques();

    if (!std::filesystem::is_directory(directorio))
        std::filesystem::create_directories(directorio);

    for (const std::filesystem::directory_entry &entrada : std::filesystem::directory_iterator(directorio))
    {
        if (entrada.is_regular_file())
        {
            const auto ext = entrada.path().extension().string();
            if (ext == ".dat" || ext == ".json")
                nombres.push_back(entrada.path().filename().string());
        }
    }

    return true;
}