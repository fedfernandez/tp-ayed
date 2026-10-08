#ifndef RUTAS_H
#define RUTAS_H

#include <filesystem>
#include <string>
#include <vector>

// Resuelve un nombre relativo dentro de la carpeta "ataques".
bool rutaEnAtaques(const std::string& nombre,
                   std::filesystem::path& ruta,
                   std::string& mensaje);

// Lista los archivos existentes dentro de la carpeta "ataques".
bool listarArchivosAtaques(std::vector<std::string>& nombres);

#endif