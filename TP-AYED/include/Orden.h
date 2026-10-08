#ifndef ORDEN_H
#define ORDEN_H

#include <cstddef>
#include <string>
#include <vector>

inline int FILAS = 10;
inline int COLUMNAS = 10;

// Orden almacenada en una celda de la cuadricula.
struct Orden
{
    unsigned int espera = 0;
    bool soltarGranada1 = false;
    bool soltarGranada2 = false;
    bool ataqueKamikaze = false;
    bool aterrizaje = false;
    bool despegue = false;
    int siguientex = 0;
    int siguientey = 0;

    bool tieneAccion() const;
    bool tieneAtaque() const;
    bool esFinal() const;
    bool sinSiguiente() const;
    std::string descripcion() const;
};

// Registro tal como se guarda en el archivo CSV.
struct OrdenArchivo
{
    int x = 0;
    int y = 0;
    unsigned int espera = 0;
    bool soltarGranada1 = false;
    bool soltarGranada2 = false;
    bool ataqueKamikaze = false;
    bool aterrizaje = false;
    bool despegue = false;
    int siguientex = 0;
    int siguientey = 0;
};

void convertir(const OrdenArchivo& origen, Orden& destino);
void convertir(int x, int y, const Orden& origen, OrdenArchivo& destino);
bool combinarOrden(Orden& destino, const Orden& nueva, std::string& mensaje);

// Cuadricula de FILAS x COLUMNAS en memoria (asignada en el heap).
class Memoria
{
public:
    Memoria();

    Orden& celda(int x, int y);
    const Orden& celda(int x, int y) const;

private:
    std::size_t indice(int x, int y) const;
    std::vector<Orden> celdas_;
};

#endif