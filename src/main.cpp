// UTN FRBA - Algoritmos y Estructuras de Datos
// TP grupal: sistema de guia de apoyo para drones (sin GPS)
//
// Convenciones del programa:
//  - Las tablas se indexan [x][y]: el primer indice es la coordenada x y el
//    segundo la coordenada y. La tabla es cuadrada (200 x 200), por eso FILAS
//    y COLUMNAS valen lo mismo.
//  - memoria[x][y] guarda la orden de la celda; ocupado[x][y] indica si esa
//    celda tiene una orden cargada (una Orden vacia es igual a una celda sin cargar).
//  - Toda la entrada por teclado se lee por lineas (leerLinea, leerEntero,
//    leerSiNo), asi un dato mal escrito nunca deja el programa en un estado raro.

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define FILAS 200
#define COLUMNAS 200
#define RUTA_MAX 256
#define LINEAS_POR_PAGINA 20

using namespace std;

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
};

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

//prototipos de funciones

// entrada por teclado
void leerLinea(const char texto[], char destino[], int tam);
int leerEntero(const char texto[], int minimo, int maximo);
bool leerSiNo(const char texto[]);

// utilidades sobre la memoria y los registros
void inicializarMemoria(Orden memoria[][COLUMNAS], bool ocupado[][COLUMNAS]);
int contarRegistros(const bool ocupado[][COLUMNAS]);
void contarGranadas(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS], int &g1, int &g2);
Orden aOrden(const OrdenArchivo &reg);
OrdenArchivo aArchivo(int x, int y, const Orden &o);
void mostrarOrden(int x, int y, const Orden &o);
int obtenerOrden(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS], int xs[], int ys[]);

// validaciones
bool validarCelda(int x, int y, const Orden &o, char mensaje[]);
bool validarAtaque(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS], char mensaje[]);
void informarValidez(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS]);

// archivos
bool leerArchivo(const char ruta[], Orden tabla[][COLUMNAS], bool ocupadoTabla[][COLUMNAS], char mensaje[], int &cantidad);
int escribirArchivo(const char ruta[], const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS]);
bool confirmarSobrescritura(const char ruta[]);

// ingreso de un registro (lo usan las operaciones 3, 4 y 5)
void pedirOrden(Orden &o, int x, int y, bool preguntarDespegue, bool esDespegue,
                bool g1Usada, bool g2Usada, const bool usadas[][COLUMNAS]);

// operaciones del menu
void menu(Orden memoria[][COLUMNAS], bool ocupado[][COLUMNAS]);
void cargarAtaque(Orden memoria[][COLUMNAS], bool ocupado[][COLUMNAS]);
void mostrarAtaque(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS]);
void crearArchivoAtaque();
void corregirArchivo();
void corregirRegistro(Orden memoria[][COLUMNAS], bool ocupado[][COLUMNAS]);
void guardarMemoria(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS]);
void visualizar(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS]);

//definiciones

// ---------------------------------------------------------------------------
// Entrada por teclado
// ---------------------------------------------------------------------------

// Lee una linea completa (admite espacios, por ejemplo en rutas de carpetas).
// Si la lectura falla, la linea se descarta y queda vacia: quien llama
// (leerEntero, leerSiNo) la toma como dato invalido y la vuelve a pedir.
void leerLinea(const char texto[], char destino[], int tam)
{
    cout << texto;
    cin.getline(destino, tam);
    if (cin.fail())
    {
        // la linea era mas larga que el espacio disponible (o se presiono Ctrl+Z)
        bool finDeEntrada = cin.eof();
        cin.clear();
        if (!finDeEntrada)
            cin.ignore(10000, '\n'); // descarta el resto de la linea larga
        destino[0] = '\0';
    }
}

// Pide un numero entero entre minimo y maximo (inclusive). Repite hasta que sea valido.
int leerEntero(const char texto[], int minimo, int maximo)
{
    char linea[100];
    char texto2[150];
    sprintf(texto2, "%s: ", texto);

    while (true)
    {
        leerLinea(texto2, linea, 100);
        char *resto;
        long valor = strtol(linea, &resto, 10);
        while (*resto == ' ' || *resto == '\t' || *resto == '\r')
            resto++;

        if (resto != linea && *resto == '\0' && valor >= minimo && valor <= maximo)
            return (int)valor;

        cout << "Valor invalido. Ingrese un numero entre " << minimo << " y " << maximo << "." << endl;
    }
}

// Pregunta por si o no. Repite hasta que la respuesta sea valida.
bool leerSiNo(const char texto[])
{
    char linea[100];
    char texto2[150];
    sprintf(texto2, "%s (s/n): ", texto);

    while (true)
    {
        leerLinea(texto2, linea, 100);
        if (linea[0] == 's' || linea[0] == 'S')
            return true;
        if (linea[0] == 'n' || linea[0] == 'N')
            return false;
        cout << "Responda s o n." << endl;
    }
}

// ---------------------------------------------------------------------------
// Utilidades sobre la memoria y los registros
// ---------------------------------------------------------------------------

void inicializarMemoria(Orden memoria[][COLUMNAS], bool ocupado[][COLUMNAS])
{
    for (int x = 0; x < COLUMNAS; x++)
    {
        for (int y = 0; y < FILAS; y++)
        {
            memoria[x][y] = Orden();
            ocupado[x][y] = false;
        }
    }
    return;
}

int contarRegistros(const bool ocupado[][COLUMNAS])
{
    int cantidad = 0;
    for (int x = 0; x < COLUMNAS; x++)
        for (int y = 0; y < FILAS; y++)
            if (ocupado[x][y])
                cantidad++;
    return cantidad;
}

// Cuenta en cuantas celdas se suelta la granada 1 y en cuantas la granada 2.
void contarGranadas(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS], int &g1, int &g2)
{
    g1 = 0;
    g2 = 0;
    for (int x = 0; x < COLUMNAS; x++)
    {
        for (int y = 0; y < FILAS; y++)
        {
            if (ocupado[x][y])
            {
                if (memoria[x][y].soltarGranada1)
                    g1++;
                if (memoria[x][y].soltarGranada2)
                    g2++;
            }
        }
    }
    return;
}

Orden aOrden(const OrdenArchivo &reg)
{
    Orden o;
    o.espera = reg.espera;
    o.soltarGranada1 = reg.soltarGranada1;
    o.soltarGranada2 = reg.soltarGranada2;
    o.ataqueKamikaze = reg.ataqueKamikaze;
    o.aterrizaje = reg.aterrizaje;
    o.despegue = reg.despegue;
    o.siguientex = reg.siguientex;
    o.siguientey = reg.siguientey;
    return o;
}

OrdenArchivo aArchivo(int x, int y, const Orden &o)
{
    OrdenArchivo reg;
    reg.x = x;
    reg.y = y;
    reg.espera = o.espera;
    reg.soltarGranada1 = o.soltarGranada1;
    reg.soltarGranada2 = o.soltarGranada2;
    reg.ataqueKamikaze = o.ataqueKamikaze;
    reg.aterrizaje = o.aterrizaje;
    reg.despegue = o.despegue;
    reg.siguientex = o.siguientex;
    reg.siguientey = o.siguientey;
    return reg;
}

// Muestra una orden en una linea, listando las acciones en el orden de
// precedencia del enunciado: despegue, aterrizaje, granada 1, granada 2,
// kamikaze, esperar y mover a la siguiente posicion.
void mostrarOrden(int x, int y, const Orden &o)
{
    const char *separador = "";

    cout << "(x=" << x << ", y=" << y << ") ";
    if (o.despegue)
    {
        cout << separador << "Despegue";
        separador = " + ";
    }
    if (o.aterrizaje)
    {
        cout << separador << "Aterrizaje";
        separador = " + ";
    }
    if (o.soltarGranada1)
    {
        cout << separador << "Granada 1";
        separador = " + ";
    }
    if (o.soltarGranada2)
    {
        cout << separador << "Granada 2";
        separador = " + ";
    }
    if (o.ataqueKamikaze)
    {
        cout << separador << "Kamikaze";
        separador = " + ";
    }
    if (o.espera > 0)
    {
        cout << separador << "Espera " << o.espera << " s";
        separador = " + ";
    }
    if (!o.aterrizaje && !o.ataqueKamikaze)
        cout << separador << "Mover a (" << o.siguientex << ", " << o.siguientey << ")";
    cout << endl;
    return;
}

// Arma la lista de celdas en el orden en que las recorre el dron: del despegue
// hasta el final. Las celdas que no forman parte de la ruta (si las hubiera)
// se agregan al final, por coordenadas. Devuelve la cantidad de celdas.
// xs e ys deben tener lugar para FILAS * COLUMNAS elementos.
int obtenerOrden(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS], int xs[], int ys[])
{
    bool visitado[FILAS][COLUMNAS] = {{false}};
    int n = 0;
    int cx = 0, cy = 0;
    bool hayDespegue = false;

    for (int x = 0; x < COLUMNAS && !hayDespegue; x++)
    {
        for (int y = 0; y < FILAS && !hayDespegue; y++)
        {
            if (ocupado[x][y] && memoria[x][y].despegue)
            {
                cx = x;
                cy = y;
                hayDespegue = true;
            }
        }
    }

    // seguir la ruta desde el despegue (se corta si hay un hueco, un repetido o el final)
    bool fin = false;
    while (hayDespegue && !fin && cx >= 0 && cx < COLUMNAS && cy >= 0 && cy < FILAS
           && ocupado[cx][cy] && !visitado[cx][cy])
    {
        visitado[cx][cy] = true;
        xs[n] = cx;
        ys[n] = cy;
        n++;
        if (memoria[cx][cy].aterrizaje || memoria[cx][cy].ataqueKamikaze)
            fin = true; // llegamos al final de la ruta
        else
        {
            int nx = memoria[cx][cy].siguientex;
            int ny = memoria[cx][cy].siguientey;
            cx = nx;
            cy = ny;
        }
    }

    // celdas sueltas
    for (int x = 0; x < COLUMNAS; x++)
    {
        for (int y = 0; y < FILAS; y++)
        {
            if (ocupado[x][y] && !visitado[x][y])
            {
                xs[n] = x;
                ys[n] = y;
                n++;
            }
        }
    }
    return n;
}

// ---------------------------------------------------------------------------
// Validaciones
// ---------------------------------------------------------------------------

// Reglas que se pueden controlar mirando una sola celda.
bool validarCelda(int x, int y, const Orden &o, char mensaje[])
{
    if (o.despegue && (o.soltarGranada1 || o.soltarGranada2 || o.ataqueKamikaze))
    {
        sprintf(mensaje, "El despegue en [%d][%d] no puede estar acompanado de ataques.", x, y);
        return false;
    }
    // el dron no puede despegar y aterrizar en la misma celda
    if (o.despegue && o.aterrizaje)
    {
        sprintf(mensaje, "La celda [%d][%d] tiene despegue y aterrizaje a la vez.", x, y);
        return false;
    }
    if (o.aterrizaje && o.ataqueKamikaze)
    {
        sprintf(mensaje, "La celda [%d][%d] tiene aterrizaje y kamikaze a la vez.", x, y);
        return false;
    }

    // Si la celda no termina la ruta, tiene que apuntar a una celda aledana
    // (las 8 vecinas: arriba, abajo, izquierda, derecha y diagonales).
    if (!o.aterrizaje && !o.ataqueKamikaze)
    {
        int nx = o.siguientex;
        int ny = o.siguientey;

        if (nx < 0 || nx >= COLUMNAS || ny < 0 || ny >= FILAS)
        {
            sprintf(mensaje, "La celda [%d][%d] apunta a [%d][%d], fuera de la tabla.", x, y, nx, ny);
            return false;
        }
        if (nx == x && ny == y)
        {
            sprintf(mensaje, "La celda [%d][%d] apunta a si misma.", x, y);
            return false;
        }
        if (abs(nx - x) > 1 || abs(ny - y) > 1)
        {
            sprintf(mensaje, "Salto invalido de [%d][%d] a [%d][%d]: no son celdas aledanas.", x, y, nx, ny);
            return false;
        }
    }
    return true;
}

// Valida TODAS las reglas del enunciado sobre una tabla ya armada.
// Devuelve true si es valida. Si no, devuelve false y deja en "mensaje" el motivo.
// No modifica la tabla.
bool validarAtaque(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS], char mensaje[])
{
    int despegues = 0, finales = 0, registros = 0;
    int granada1 = 0, granada2 = 0;
    int cx = -1, cy = -1; // celda de despegue

    // Paso 1: reglas de cada celda y conteos
    for (int x = 0; x < COLUMNAS; x++)
    {
        for (int y = 0; y < FILAS; y++)
        {
            if (ocupado[x][y])
            {
                registros++;
                const Orden &o = memoria[x][y];

                if (!validarCelda(x, y, o, mensaje))
                    return false;

                if (o.despegue)
                {
                    despegues++;
                    cx = x;
                    cy = y;
                }
                if (o.aterrizaje || o.ataqueKamikaze)
                    finales++;
                if (o.soltarGranada1)
                    granada1++;
                if (o.soltarGranada2)
                    granada2++;
            }
        }
    }

    if (registros == 0)
    {
        sprintf(mensaje, "El archivo no tiene registros.");
        return false;
    }
    if (despegues != 1)
    {
        sprintf(mensaje, "Debe haber exactamente 1 despegue (hay %d).", despegues);
        return false;
    }
    if (finales != 1)
    {
        sprintf(mensaje, "Debe haber exactamente 1 aterrizaje o kamikaze (hay %d).", finales);
        return false;
    }
    // el dron lleva dos granadas: la 1 y la 2, cada una se suelta una sola vez
    if (granada1 > 1)
    {
        sprintf(mensaje, "La granada 1 se suelta en %d celdas (maximo 1).", granada1);
        return false;
    }
    if (granada2 > 1)
    {
        sprintf(mensaje, "La granada 2 se suelta en %d celdas (maximo 1).", granada2);
        return false;
    }

    // Paso 2: caminar la ruta desde el despegue hasta el final
    bool visitado[FILAS][COLUMNAS] = {{false}};
    int recorridas = 0;
    bool fin = false;

    while (!fin)
    {
        visitado[cx][cy] = true;
        recorridas++;

        if (memoria[cx][cy].aterrizaje || memoria[cx][cy].ataqueKamikaze)
            fin = true; // llegamos al final de la ruta
        else
        {
            // validarCelda ya garantizo que el siguiente esta dentro de la tabla y es aledano
            int nx = memoria[cx][cy].siguientex;
            int ny = memoria[cx][cy].siguientey;

            if (!ocupado[nx][ny])
            {
                sprintf(mensaje, "Hueco en la ruta: [%d][%d] apunta a [%d][%d], que no tiene registro.", cx, cy, nx, ny);
                return false;
            }
            if (visitado[nx][ny])
            {
                sprintf(mensaje, "La ruta pasa mas de una vez por [%d][%d].", nx, ny);
                return false;
            }
            cx = nx;
            cy = ny;
        }
    }

    // Si hay mas registros que celdas recorridas, quedaron registros fuera de la ruta
    if (recorridas != registros)
    {
        sprintf(mensaje, "Hay %d registro(s) que no forman parte de la ruta del despegue al final.", registros - recorridas);
        return false;
    }
    return true;
}

// Avisa si la tabla en memoria es valida o no (se usa despues de corregir).
void informarValidez(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS])
{
    char mensaje[200];
    if (validarAtaque(memoria, ocupado, mensaje))
        cout << "El ataque quedo valido." << endl;
    else
        cout << "ATENCION: el ataque todavia no es valido. " << mensaje << endl;
    return;
}

// ---------------------------------------------------------------------------
// Archivos
// ---------------------------------------------------------------------------

// Lee un archivo de ataque en "tabla" y controla lo que depende del formato:
// que tenga registros completos, que las coordenadas esten dentro de la tabla
// y que no haya dos registros para la misma celda. Las reglas del ataque se
// controlan despues con validarAtaque.
bool leerArchivo(const char ruta[], Orden tabla[][COLUMNAS], bool ocupadoTabla[][COLUMNAS], char mensaje[], int &cantidad)
{
    cantidad = 0;

    FILE *archivo = fopen(ruta, "rb");
    if (archivo == NULL)
    {
        sprintf(mensaje, "No se pudo abrir el archivo.");
        return false;
    }

    // el archivo debe contener registros completos
    fseek(archivo, 0, SEEK_END);
    long tamanio = ftell(archivo);
    rewind(archivo);
    if (tamanio % (long)sizeof(OrdenArchivo) != 0)
    {
        sprintf(mensaje, "El tamano del archivo no corresponde a registros completos.");
        fclose(archivo);
        return false;
    }

    inicializarMemoria(tabla, ocupadoTabla);

    OrdenArchivo reg;
    while (fread(&reg, sizeof(OrdenArchivo), 1, archivo) == 1)
    {
        cantidad++;
        if (reg.x < 0 || reg.x >= COLUMNAS || reg.y < 0 || reg.y >= FILAS)
        {
            sprintf(mensaje, "Registro %d: la coordenada [%d][%d] esta fuera de la tabla (0 a %d).",
                    cantidad, reg.x, reg.y, COLUMNAS - 1);
            fclose(archivo);
            return false;
        }
        if (ocupadoTabla[reg.x][reg.y])
        {
            sprintf(mensaje, "Registro %d: la celda [%d][%d] esta repetida en el archivo.", cantidad, reg.x, reg.y);
            fclose(archivo);
            return false;
        }
        tabla[reg.x][reg.y] = aOrden(reg);
        ocupadoTabla[reg.x][reg.y] = true;
    }

    fclose(archivo);
    return true;
}

// Escribe la tabla en un archivo binario. Los registros se escriben en el orden
// del recorrido (despegue -> final). Devuelve la cantidad de registros
// escritos, o -1 si no se pudo crear el archivo.
int escribirArchivo(const char ruta[], const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS])
{
    static int xs[FILAS * COLUMNAS];
    static int ys[FILAS * COLUMNAS];
    int n = obtenerOrden(memoria, ocupado, xs, ys);

    FILE *archivo = fopen(ruta, "wb");
    if (archivo == NULL)
        return -1;

    for (int i = 0; i < n; i++)
    {
        OrdenArchivo reg = aArchivo(xs[i], ys[i], memoria[xs[i]][ys[i]]);
        fwrite(&reg, sizeof(OrdenArchivo), 1, archivo);
    }
    fclose(archivo);
    return n;
}

// Si el archivo ya existe pregunta si se puede sobrescribir.
bool confirmarSobrescritura(const char ruta[])
{
    FILE *archivo = fopen(ruta, "rb");
    if (archivo == NULL)
        return true;
    fclose(archivo);
    return leerSiNo("El archivo ya existe. Desea sobrescribirlo");
}

// ---------------------------------------------------------------------------
// Ingreso de un registro (operaciones 3, 4 y 5)
// ---------------------------------------------------------------------------

// Pide por teclado todos los datos de la orden de la celda (x, y), en el orden
// de precedencia del enunciado. Controla las reglas de la celda y vuelve a
// pedir los datos si no se cumplen.
//  preguntarDespegue: si es true se pregunta si es el despegue; si es false
//                     la celda es despegue o no segun "esDespegue".
//  g1Usada / g2Usada: esa granada ya se suelta en OTRA celda, no se puede repetir.
//  usadas:            celdas que ya forman parte de la ruta (solo al crear un
//                     archivo, para no pasar dos veces por la misma celda). Puede ser NULL.
void pedirOrden(Orden &o, int x, int y, bool preguntarDespegue, bool esDespegue,
                bool g1Usada, bool g2Usada, const bool usadas[][COLUMNAS])
{
    char mensaje[200];
    bool correcta;

    do
    {
        correcta = true;
        o = Orden();

        if (preguntarDespegue)
            o.despegue = leerSiNo("Es la celda de despegue");
        else
            o.despegue = esDespegue;

        if (o.despegue)
        {
            cout << "Celda de despegue: no puede aterrizar, soltar granadas ni hacer kamikaze." << endl;
        }
        else
        {
            o.aterrizaje = leerSiNo("Aterriza en esta celda");
        }

        if (!o.despegue)
        {
            if (g1Usada)
                cout << "(La granada 1 ya se suelta en otra celda.)" << endl;
            else
                o.soltarGranada1 = leerSiNo("Suelta la granada 1");

            if (g2Usada)
                cout << "(La granada 2 ya se suelta en otra celda.)" << endl;
            else
                o.soltarGranada2 = leerSiNo("Suelta la granada 2");

            o.ataqueKamikaze = leerSiNo("Hace ataque kamikaze");
        }

        o.espera = (unsigned int)leerEntero("Segundos de espera (0 si no espera)", 0, 100000);

        if (!o.aterrizaje && !o.ataqueKamikaze)
        {
            o.siguientex = leerEntero("Siguiente x", 0, COLUMNAS - 1);
            o.siguientey = leerEntero("Siguiente y", 0, FILAS - 1);
        }

        if (!validarCelda(x, y, o, mensaje))
        {
            cout << "Error: " << mensaje << " Vuelva a ingresar los datos." << endl;
            correcta = false;
        }
        else if (usadas != NULL && !o.aterrizaje && !o.ataqueKamikaze
                 && usadas[o.siguientex][o.siguientey])
        {
            cout << "Error: la celda [" << o.siguientex << "][" << o.siguientey
                 << "] ya forma parte de la ruta (no se puede pasar dos veces). Vuelva a ingresar los datos." << endl;
            correcta = false;
        }
    } while (!correcta);

    return;
}

// ---------------------------------------------------------------------------
// Menu
// ---------------------------------------------------------------------------

void menu(Orden memoria[][COLUMNAS], bool ocupado[][COLUMNAS])
{
    int opcion;
    do
    {
        cout << endl;
        cout << "1- Cargar archivo de ataque en memoria." << endl;
        cout << "2- Mostrar un ataque cargado." << endl;
        cout << "3- Crear un archivo de ataque nuevo." << endl;
        cout << "4- Corregir un registro del archivo." << endl;
        cout << "5- Corregir un registro en memoria." << endl;
        cout << "6- Guardar memoria en un archivo nuevo." << endl;
        cout << "7- Visualizar un archivo de ataque en html." << endl;
        cout << "8- Salir." << endl;
        opcion = leerEntero("Ingrese una opcion", 1, 8);
        cout << endl;

        switch (opcion)
        {
            case 1: cargarAtaque(memoria, ocupado); break;
            case 2: mostrarAtaque(memoria, ocupado); break;
            case 3: crearArchivoAtaque(); break;
            case 4: corregirArchivo(); break;
            case 5: corregirRegistro(memoria, ocupado); break;
            case 6: guardarMemoria(memoria, ocupado); break;
            case 7: visualizar(memoria, ocupado); break;
            case 8: cout << "Saliendo..." << endl; break;
        }
    } while (opcion != 8);
    return;
}

// ---------------------------------------------------------------------------
// Operacion 1: cargar archivo de ataque en memoria
// ---------------------------------------------------------------------------

void cargarAtaque(Orden memoria[][COLUMNAS], bool ocupado[][COLUMNAS])
{
    char ruta[RUTA_MAX];
    leerLinea("Ingrese la ruta y nombre del archivo: ", ruta, RUTA_MAX);

    // Se carga en una tabla TEMPORAL: si el archivo es invalido, la memoria
    // que ya estaba cargada no se toca ("no cargar el archivo").
    static Orden temp[FILAS][COLUMNAS];
    static bool tempOcupado[FILAS][COLUMNAS];
    char mensaje[200] = "";
    int cantidad = 0;

    if (!leerArchivo(ruta, temp, tempOcupado, mensaje, cantidad)
        || !validarAtaque(temp, tempOcupado, mensaje))
    {
        cout << "Archivo NO cargado. " << mensaje << endl;
        return;
    }

    // Todo bien: recien ahora se copia a la memoria real
    for (int x = 0; x < COLUMNAS; x++)
    {
        for (int y = 0; y < FILAS; y++)
        {
            memoria[x][y] = temp[x][y];
            ocupado[x][y] = tempOcupado[x][y];
        }
    }
    cout << "Archivo cargado y validado correctamente (" << cantidad << " registros)." << endl;
    return;
}

// ---------------------------------------------------------------------------
// Operacion 2: mostrar ataque
// ---------------------------------------------------------------------------

void mostrarAtaque(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS])
{
    static int xs[FILAS * COLUMNAS];
    static int ys[FILAS * COLUMNAS];
    int n = obtenerOrden(memoria, ocupado, xs, ys);

    if (n == 0)
    {
        cout << "No hay ningun ataque cargado en memoria." << endl;
        return;
    }

    cout << "ATAQUE CARGADO EN MEMORIA (" << n << " registros, en orden de recorrido)" << endl;

    char mensaje[200];
    if (!validarAtaque(memoria, ocupado, mensaje))
        cout << "ATENCION: el ataque no es valido. " << mensaje << endl;
    cout << endl;

    int lineasImpresas = 0;
    for (int i = 0; i < n; i++)
    {
        cout << "Paso " << (i + 1) << ": ";
        mostrarOrden(xs[i], ys[i], memoria[xs[i]][ys[i]]);
        lineasImpresas++;

        // Cuidar de que no se exceda el tamano de pantalla: una pausa cada pagina
        if (lineasImpresas == LINEAS_POR_PAGINA && i < n - 1)
        {
            cout << endl << "-- Presione una tecla para ver la siguiente pagina --" << endl;
            getch();
            cout << endl;
            lineasImpresas = 0;
        }
    }
    cout << endl << "Fin del ataque." << endl;
    return;
}

// ---------------------------------------------------------------------------
// Operacion 3: crear un archivo de ataque nuevo
// ---------------------------------------------------------------------------

void crearArchivoAtaque()
{
    char ruta[RUTA_MAX];
    leerLinea("Ingrese ruta y nombre del archivo a crear: ", ruta, RUTA_MAX);

    if (!confirmarSobrescritura(ruta))
    {
        cout << "No se creo el archivo." << endl;
        return;
    }

    // comprobar desde el principio que se puede escribir en esa ruta
    // (modo "ab": no borra el contenido de un archivo existente)
    FILE *prueba = fopen(ruta, "ab");
    if (prueba == NULL)
    {
        cout << "No se pudo crear el archivo." << endl;
        return;
    }
    fclose(prueba);

    // El ataque se arma en una tabla y recien al final se guarda en el archivo
    static Orden temp[FILAS][COLUMNAS];
    static bool tempOcupado[FILAS][COLUMNAS];
    inicializarMemoria(temp, tempOcupado);

    cout << "Celda de despegue del dron" << endl;
    int x = leerEntero("Coordenada x", 0, COLUMNAS - 1);
    int y = leerEntero("Coordenada y", 0, FILAS - 1);

    int registros = 0;
    bool granada1Usada = false;
    bool granada2Usada = false;
    bool fin = false;

    while (!fin)
    {
        cout << endl << "Registro " << (registros + 1) << " - celda [" << x << "][" << y << "]" << endl;

        Orden o;
        // el primer registro es siempre el despegue; los demas, nunca
        pedirOrden(o, x, y, false, registros == 0, granada1Usada, granada2Usada, tempOcupado);

        temp[x][y] = o;
        tempOcupado[x][y] = true;
        registros++;

        if (o.soltarGranada1)
            granada1Usada = true;
        if (o.soltarGranada2)
            granada2Usada = true;

        if (o.aterrizaje || o.ataqueKamikaze)
            fin = true; // aterrizar o kamikaze termina el recorrido
        else
        {
            x = o.siguientex;
            y = o.siguientey;
        }
    }

    // Control final (no deberia fallar porque cada registro se controla al ingresarlo)
    char mensaje[200];
    if (!validarAtaque(temp, tempOcupado, mensaje))
    {
        cout << "No se creo el archivo. " << mensaje << endl;
        return;
    }

    int escritos = escribirArchivo(ruta, temp, tempOcupado);
    if (escritos < 0)
    {
        cout << "No se pudo crear el archivo." << endl;
        return;
    }
    cout << endl << "Archivo creado con exito. Total registros: " << escritos << endl;
    return;
}

// ---------------------------------------------------------------------------
// Operacion 4: corregir un registro del archivo
// ---------------------------------------------------------------------------

void corregirArchivo()
{
    char ruta[RUTA_MAX];
    leerLinea("Ingrese la ruta y nombre del archivo a corregir: ", ruta, RUTA_MAX);

    FILE *f = fopen(ruta, "rb+");
    if (f == NULL)
    {
        cout << "No se pudo abrir el archivo." << endl;
        return;
    }

    int x = leerEntero("Coordenada x del registro a corregir", 0, COLUMNAS - 1);
    int y = leerEntero("Coordenada y del registro a corregir", 0, FILAS - 1);

    // Buscar el registro y, de paso, contar las granadas del archivo
    OrdenArchivo reg, encontrado;
    long posicion = -1, indice = 0;
    int coincidencias = 0, totalG1 = 0, totalG2 = 0;

    while (fread(&reg, sizeof(OrdenArchivo), 1, f) == 1)
    {
        if (reg.soltarGranada1)
            totalG1++;
        if (reg.soltarGranada2)
            totalG2++;
        if (reg.x == x && reg.y == y)
        {
            coincidencias++;
            if (posicion < 0)
            {
                posicion = indice;
                encontrado = reg;
            }
        }
        indice++;
    }

    if (posicion < 0)
    {
        fclose(f);
        cout << "El archivo no tiene ningun registro en la celda [" << x << "][" << y << "]." << endl;
        return;
    }
    if (coincidencias > 1)
        cout << "Aviso: hay " << coincidencias << " registros para esa celda. Se corrige el primero." << endl;

    cout << "Registro actual: ";
    mostrarOrden(x, y, aOrden(encontrado));

    if (!leerSiNo("Desea ingresar nuevamente los datos"))
    {
        fclose(f);
        cout << "No se modifico nada." << endl;
        return;
    }

    // las granadas que ya se sueltan en OTROS registros no se pueden repetir
    bool g1Usada = (totalG1 - (encontrado.soltarGranada1 ? 1 : 0)) > 0;
    bool g2Usada = (totalG2 - (encontrado.soltarGranada2 ? 1 : 0)) > 0;

    Orden nueva;
    pedirOrden(nueva, x, y, true, encontrado.despegue, g1Usada, g2Usada, NULL);

    // el registro se reemplaza en el mismo lugar del archivo
    OrdenArchivo nuevoReg = aArchivo(x, y, nueva);
    fseek(f, posicion * (long)sizeof(OrdenArchivo), SEEK_SET);
    fwrite(&nuevoReg, sizeof(OrdenArchivo), 1, f);
    fclose(f);
    cout << "Registro reemplazado." << endl;

    // Se avisa si el archivo quedo valido (puede que haga falta corregir mas registros)
    static Orden temp[FILAS][COLUMNAS];
    static bool tempOcupado[FILAS][COLUMNAS];
    char mensaje[200];
    int cantidad;
    if (leerArchivo(ruta, temp, tempOcupado, mensaje, cantidad) && validarAtaque(temp, tempOcupado, mensaje))
        cout << "El archivo quedo valido." << endl;
    else
        cout << "ATENCION: el archivo todavia no es valido. " << mensaje << endl;
    return;
}

// ---------------------------------------------------------------------------
// Operacion 5: corregir un registro en memoria
// ---------------------------------------------------------------------------

void corregirRegistro(Orden memoria[][COLUMNAS], bool ocupado[][COLUMNAS])
{
    if (contarRegistros(ocupado) == 0)
    {
        cout << "No hay ningun ataque cargado en memoria." << endl;
        return;
    }

    int x = leerEntero("Coordenada x del registro a corregir", 0, COLUMNAS - 1);
    int y = leerEntero("Coordenada y del registro a corregir", 0, FILAS - 1);

    if (!ocupado[x][y])
    {
        cout << "La celda [" << x << "][" << y << "] no tiene ningun registro cargado." << endl;
        return;
    }

    cout << "Registro actual: ";
    mostrarOrden(x, y, memoria[x][y]);

    if (!leerSiNo("Desea ingresar nuevamente los datos"))
    {
        cout << "No se modifico nada." << endl;
        return;
    }

    // las granadas que ya se sueltan en OTRAS celdas no se pueden repetir
    int g1, g2;
    contarGranadas(memoria, ocupado, g1, g2);
    bool g1Usada = (g1 - (memoria[x][y].soltarGranada1 ? 1 : 0)) > 0;
    bool g2Usada = (g2 - (memoria[x][y].soltarGranada2 ? 1 : 0)) > 0;

    Orden nueva;
    pedirOrden(nueva, x, y, true, memoria[x][y].despegue, g1Usada, g2Usada, NULL);

    memoria[x][y] = nueva;
    cout << "Registro reemplazado." << endl;

    // Se avisa si el ataque quedo valido (puede que haga falta corregir mas registros)
    informarValidez(memoria, ocupado);
    return;
}

// ---------------------------------------------------------------------------
// Operacion 6: guardar memoria en un archivo nuevo
// ---------------------------------------------------------------------------

void guardarMemoria(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS])
{
    if (contarRegistros(ocupado) == 0)
    {
        cout << "La memoria esta vacia, no hay nada para guardar." << endl;
        return;
    }

    char mensaje[200];
    if (!validarAtaque(memoria, ocupado, mensaje))
    {
        cout << "ATENCION: el ataque en memoria no es valido. " << mensaje << endl;
        if (!leerSiNo("Desea guardarlo igual"))
        {
            cout << "No se guardo el archivo." << endl;
            return;
        }
    }

    char ruta[RUTA_MAX];
    leerLinea("Ingrese ruta y nombre del archivo: ", ruta, RUTA_MAX);

    if (!confirmarSobrescritura(ruta))
    {
        cout << "No se guardo el archivo." << endl;
        return;
    }

    int escritos = escribirArchivo(ruta, memoria, ocupado);
    if (escritos < 0)
        cout << "No se pudo crear el archivo." << endl;
    else
        cout << "Memoria guardada con exito. Total registros: " << escritos << endl;
    return;
}

// ---------------------------------------------------------------------------
// Operacion 7: visualizar el ataque en html
// ---------------------------------------------------------------------------

void visualizar(const Orden memoria[][COLUMNAS], const bool ocupado[][COLUMNAS])
{
    int registros = contarRegistros(ocupado);
    if (registros == 0)
    {
        cout << "No hay ningun ataque cargado en memoria." << endl;
        return;
    }

    char ruta[RUTA_MAX];
    leerLinea("Ingrese ruta y nombre del archivo HTML: ", ruta, RUTA_MAX);

    // si no tiene extension .html / .htm se la agregamos
    int largo = (int)strlen(ruta);
    bool tieneExtension = (largo >= 5 && strcmp(ruta + largo - 5, ".html") == 0)
                          || (largo >= 4 && strcmp(ruta + largo - 4, ".htm") == 0);
    if (!tieneExtension && largo > 0 && largo + 5 < RUTA_MAX)
        strcat(ruta, ".html");

    if (!confirmarSobrescritura(ruta))
    {
        cout << "No se genero el archivo." << endl;
        return;
    }

    FILE *archivo = fopen(ruta, "w");
    if (archivo == NULL)
    {
        cout << "No se pudo crear el archivo." << endl;
        return;
    }

    fprintf(archivo, "<html>\n");
    fprintf(archivo, "<head>\n");
    fprintf(archivo, "<title>Mapa de Ataque</title>\n");
    fprintf(archivo, "<style>\n");
    fprintf(archivo, "body { font-family: Arial, sans-serif; background-color: #222; color: #fff; padding: 10px; }\n");
    fprintf(archivo, "table { border-collapse: collapse; }\n");
    fprintf(archivo, "td { width: 18px; height: 18px; text-align: center; font-size: 10px; font-weight: bold; border: 1px solid #444; }\n");
    fprintf(archivo, ".despegue { background-color: #ffeb3b; color: #000; }\n");
    fprintf(archivo, ".aterrizaje { background-color: #03a9f4; color: #fff; }\n");
    fprintf(archivo, ".kamikaze { background-color: #f44336; color: #fff; }\n");
    fprintf(archivo, ".granada { background-color: #ff9800; color: #000; }\n");
    fprintf(archivo, ".espera { background-color: #9c27b0; color: #fff; }\n");
    fprintf(archivo, ".movimiento { background-color: #4caf50; color: #fff; }\n");
    fprintf(archivo, ".vacia { background-color: #111; color: #333; }\n");
    fprintf(archivo, ".leyenda { margin-bottom: 10px; font-size: 14px; }\n");
    fprintf(archivo, ".item { display: inline-block; margin-right: 15px; }\n");
    fprintf(archivo, "</style>\n");
    fprintf(archivo, "</head>\n");
    fprintf(archivo, "<body>\n");

    fprintf(archivo, "<h2>Mapa de Ataque (200x200)</h2>\n");
    fprintf(archivo, "<p>Registros cargados: %d. Columnas = x, filas = y (ambas de 0 a 199). Pase el mouse sobre una celda para ver sus coordenadas.</p>\n", registros);
    fprintf(archivo, "<div class=\"leyenda\">\n");
    fprintf(archivo, "<span class=\"item\"><b style=\"color:#ffeb3b;\">[D]</b> Despegue</span>\n");
    fprintf(archivo, "<span class=\"item\"><b style=\"color:#03a9f4;\">[A]</b> Aterrizaje</span>\n");
    fprintf(archivo, "<span class=\"item\"><b style=\"color:#f44336;\">[K]</b> Kamikaze</span>\n");
    fprintf(archivo, "<span class=\"item\"><b style=\"color:#ff9800;\">[G1/G2]</b> Granada</span>\n");
    fprintf(archivo, "<span class=\"item\"><b style=\"color:#9c27b0;\">[E]</b> Espera</span>\n");
    fprintf(archivo, "<span class=\"item\"><b style=\"color:#4caf50;\">[M]</b> Movimiento</span>\n");
    fprintf(archivo, "</div>\n");

    fprintf(archivo, "<table border=\"1\">\n");
    for (int y = 0; y < FILAS; y++)
    {
        fprintf(archivo, "<tr>\n");
        for (int x = 0; x < COLUMNAS; x++)
        {
            if (ocupado[x][y])
            {
                if (memoria[x][y].despegue)
                    fprintf(archivo, "<td class=\"despegue\" title=\"[%d][%d] Despegue\">D</td>", x, y);
                else if (memoria[x][y].ataqueKamikaze)
                    fprintf(archivo, "<td class=\"kamikaze\" title=\"[%d][%d] Kamikaze\">K</td>", x, y);
                else if (memoria[x][y].aterrizaje)
                    fprintf(archivo, "<td class=\"aterrizaje\" title=\"[%d][%d] Aterrizaje\">A</td>", x, y);
                else if (memoria[x][y].soltarGranada1)
                    fprintf(archivo, "<td class=\"granada\" title=\"[%d][%d] Granada 1\">G1</td>", x, y);
                else if (memoria[x][y].soltarGranada2)
                    fprintf(archivo, "<td class=\"granada\" title=\"[%d][%d] Granada 2\">G2</td>", x, y);
                else if (memoria[x][y].espera > 0)
                    fprintf(archivo, "<td class=\"espera\" title=\"[%d][%d] Espera %u s\">E</td>", x, y, memoria[x][y].espera);
                else
                    fprintf(archivo, "<td class=\"movimiento\" title=\"[%d][%d] Movimiento\">M</td>", x, y);
            }
            else
            {
                fprintf(archivo, "<td class=\"vacia\">&nbsp;</td>");
            }
        }
        fprintf(archivo, "</tr>\n");
    }
    fprintf(archivo, "</table>\n");
    fprintf(archivo, "</body>\n");
    fprintf(archivo, "</html>\n");

    fclose(archivo);
    cout << "Archivo HTML generado con exito: " << ruta << endl;
    return;
}

int main()
{
    /*static: las tablas pesan ~800 KB y como variables locales normales
    pueden desbordar la pila y cerrar el programa. */
    static Orden memoria[FILAS][COLUMNAS];
    static bool ocupado[FILAS][COLUMNAS];

    inicializarMemoria(memoria, ocupado);
    menu(memoria, ocupado);
    return 0;
}
