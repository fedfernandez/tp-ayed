#include "Orden.h"

#include <sstream>

Memoria::Memoria() : celdas_(static_cast<std::size_t>(FILAS) * COLUMNAS)
{
}

std::size_t Memoria::indice(int x, int y) const
{
    return static_cast<std::size_t>(y) * COLUMNAS + x;
}

Orden &Memoria::celda(int x, int y)
{
    return celdas_[indice(x, y)];
}

const Orden &Memoria::celda(int x, int y) const
{
    return celdas_[indice(x, y)];
}

bool Orden::tieneAccion() const
{
    return espera > 0 || soltarGranada1 || soltarGranada2 || ataqueKamikaze || aterrizaje || despegue || siguientex != 0 || siguientey != 0;
}

bool Orden::tieneAtaque() const
{
    return soltarGranada1 || soltarGranada2 || ataqueKamikaze;
}

bool Orden::esFinal() const
{
    return aterrizaje || ataqueKamikaze;
}

bool Orden::sinSiguiente() const
{
    return siguientex == 0 && siguientey == 0;
}

// Las acciones se muestran en el orden de precedencia de la consigna.
std::string Orden::descripcion() const
{
    std::ostringstream texto;
    bool primera = true;

    auto agregar = [&](const std::string &parte)
    {
        if (!primera)
            texto << " | ";
        texto << parte;
        primera = false;
    };

    if (despegue)
        agregar("DESPEGUE");
    if (aterrizaje)
        agregar("ATERRIZAJE");
    if (soltarGranada1)
        agregar("GRANADA 1");
    if (soltarGranada2)
        agregar("GRANADA 2");
    if (ataqueKamikaze)
        agregar("KAMIKAZE");
    if (espera > 0)
        agregar("ESPERAR " + std::to_string(espera));
    if (!esFinal() && !sinSiguiente())
        agregar("MOVER A (" + std::to_string(siguientex) + "," + std::to_string(siguientey) + ")");

    if (primera)
        texto << "VACIA";

    return texto.str();
}

void convertir(const OrdenArchivo &origen, Orden &destino)
{
    destino.espera = origen.espera;
    destino.soltarGranada1 = origen.soltarGranada1;
    destino.soltarGranada2 = origen.soltarGranada2;
    destino.ataqueKamikaze = origen.ataqueKamikaze;
    destino.aterrizaje = origen.aterrizaje;
    destino.despegue = origen.despegue;
    destino.siguientex = origen.siguientex;
    destino.siguientey = origen.siguientey;
}

void convertir(int x, int y, const Orden &origen, OrdenArchivo &destino)
{
    destino.x = x;
    destino.y = y;
    destino.espera = origen.espera;
    destino.soltarGranada1 = origen.soltarGranada1;
    destino.soltarGranada2 = origen.soltarGranada2;
    destino.ataqueKamikaze = origen.ataqueKamikaze;
    destino.aterrizaje = origen.aterrizaje;
    destino.despegue = origen.despegue;
    destino.siguientex = origen.siguientex;
    destino.siguientey = origen.siguientey;
}

bool combinarOrden(Orden &destino, const Orden &nueva, std::string &mensaje)
{
    destino.espera += nueva.espera;
    destino.soltarGranada1 = destino.soltarGranada1 || nueva.soltarGranada1;
    destino.soltarGranada2 = destino.soltarGranada2 || nueva.soltarGranada2;
    destino.ataqueKamikaze = destino.ataqueKamikaze || nueva.ataqueKamikaze;
    destino.aterrizaje = destino.aterrizaje || nueva.aterrizaje;
    destino.despegue = destino.despegue || nueva.despegue;

    const bool destinoSeMueve = !destino.sinSiguiente();
    const bool nuevaSeMueve = !nueva.sinSiguiente();

    if (destinoSeMueve && nuevaSeMueve &&
        (destino.siguientex != nueva.siguientex ||
         destino.siguientey != nueva.siguientey))
    {
        mensaje = "Las ordenes sobre la misma celda indican posiciones siguientes distintas.";
        return false;
    }

    if (!destinoSeMueve && nuevaSeMueve)
    {
        destino.siguientex = nueva.siguientex;
        destino.siguientey = nueva.siguientey;
    }

    return true;
}