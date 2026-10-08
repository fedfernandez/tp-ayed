# Resumen del Proyecto: Sistema de Guía de Drones

## Descripción General

Aplicación de consola en C++17 para gestionar rutas de ataque de drones. Permite crear, cargar, modificar, validar y visualizar archivos de ataque en una cuadrícula configurable. El proyecto está desarrollado como un trabajo práctico académico.

**Ejecutable:** `bin/Debug/fede.exe` (compilado con CodeBlocks/MinGW)

---

## Arquitectura y Estructura de Archivos

```
fede/
├── include/                 # Headers
│   ├── Orden.h             # Estructuras de datos: Orden, OrdenArchivo, Memoria
│   ├── Validaciones.h      # Validaciones de coordenadas y lógica del ataque
│   ├── Rutas.h             # Resolución de rutas dentro de carpeta "ataques"
│   ├── ArchivoAtaque.h     # I/O de archivos (CSV y JSON)
│   ├── HTML.h              # Generación de visualización HTML
│   └── UI.h                # Interfaz de usuario por consola
├── src/                    # Implementación
│   ├── main.cpp            # Bucle principal y menú (8 opciones)
│   ├── Orden.cpp           # Lógica de órdenes, combinación, conversión
│   ├── Validaciones.cpp    # Validación completa del ataque (rutas, adyacencia, unicidad)
│   ├── Rutas.cpp           # Búsqueda de carpeta "ataques" y listado
│   ├── ArchivoAtaque.cpp   # Parsers CSV/JSON, escritores JSON
│   ├── HTML.cpp            # Generador HTML con CSS embebido
│   └── UI.cpp              # Menús, entrada de datos, visualización cuadrícula
├── ataques/                # Carpeta de datos (auto-creada)
│   ├── ejemplo.json/.dat   # Ejemplos de ataques válidos
│   ├── ataque1.json        # Otro ejemplo
│   └── *.html              # Salidas HTML generadas
├── fede.cbp                # Proyecto CodeBlocks
├── .vscode/tasks.json      # Tareas de compilación
└── .vscode/launch.json     # Configuración de depuración
```

---

## Lógica Principal

### 1. Modelo de Datos (`Orden.h` / `Orden.cpp`)

**Estructuras:**
- `Orden` - Representa una celda en memoria con todas las acciones posibles
- `OrdenArchivo` - Representación plana para serialización (CSV/JSON)
- `Memoria` - Contenedor `vector<Orden>` de tamaño `FILAS x COLUMNAS` (heap)

**Acciones por celda (precedencia):**
1. `despegue` - Inicio obligatorio (exactamente 1)
2. `aterrizaje` - Fin obligatorio (exclusivo con kamikaze)
3. `ataqueKamikaze` - Fin alternativo (exclusivo con aterrizaje)
4. `soltarGranada1` / `soltarGranada2` - Ataques intermedios
5. `espera` - Tiempo de espera (unsigned int)
5. `siguientex/y` - Movimiento a celda adyacente (8 direcciones)

**Funciones clave:**
- `convertir()` - Entre `OrdenArchivo` ↔ `Orden`
- `combinarOrden()` - Fusiona dos órdenes en la misma celda (suma esperas, OR lógico en booleanos, valida siguiente coherente)

### 2. Validaciones (`Validaciones.h` / `Validaciones.cpp`)

`validarMemoria()` ejecuta **validación completa** del ataque:

| Regla | Descripción |
|-------|-------------|
| Registros > 0 | Al menos una orden |
| Despegue = 1 | Exactamente un despegue |
| Fin = 1 | Exactamente un fin (aterrizaje XOR kamikaze) |
| Despegue limpio | Sin ataques acompañando al despegue |
| Fin sin siguiente | Aterrizaje/Kamikaze no tienen siguiente |
| Ruta continua | Desde despegue hasta fin, cada celda tiene acción |
| Sin huecos | No celdas vacías en el camino |
| Sin ciclos | No visitar misma celda dos veces |
| Adyacencia | Cada paso a celda vecina (8-dir) |
| Completa | Todos los registros forman parte de la ruta única |

### 3. Persistencia (`ArchivoAtaque.cpp`)

**Formatos soportados:**
- **CSV** (lectura): Cabecera fija `x,y,espera,bomba1,bomba2,kamikaze,aterrizaje,despegue,siguiente_x,siguiente_y`
- **JSON** (lectura/escritura): Estructura con `filas`, `columnas`, `registros[]`

**Detección automática:** Por extensión (`.json`) o inspección de primera línea (`{`)

**Escritura:** Siempre genera JSON (convierte `.dat`/`.txt` a `.json`)

### 4. Rutas (`Rutas.cpp`)

- `directorioAtaques()`: Busca carpeta `ataques` desde CWD hacia arriba (permite ejecutar desde `bin/Debug/`)
- `rutaEnAtaques()`: Valida que la ruta se quede dentro de `ataques/` (prevención path traversal)
- `listarArchivosAtaques()`: Filtra `.dat` y `.json`

### 5. Interfaz (`UI.cpp` / `main.cpp`)

**Menú principal (8 opciones):**
1. **Cargar archivo** → Lee CSV/JSON, valida, carga en `memoria` global
2. **Mostrar ataque** → Cuadrícula ASCII + lista detallada (paginado 20 items)
3. **Crear ataque** → Asistido paso a paso, valida al final, guarda opcional
4. **Corregir archivo** → Edita registro en disco, revalida completo
5. **Corregir memoria** → Edita registro en RAM, valida
6. **Guardar memoria** → Escribe JSON (valida antes)
7. **Visualizar HTML** → Genera tabla HTML con CSS embebido
8. **Configurar grilla** → Cambia `FILAS`/`COLUMNAS` (1-1000), reinicia memoria

---

## Flujo de Datos Típico

```
Usuario → Menú → [Cargar] → leerArchivo() → [CSV/JSON Parser] → construirMemoria()
                                                         ↓
                                                  validarMemoria()
                                                         ↓
                                                   memoria global
                                                         ↓
                              [Mostrar] ← mostrarCuadricula() / mostrarMemoria()
                              [Guardar] → escribirArchivo() → JSON en ataques/
                              [HTML]    → generarHTML()     → .html en ataques/
```

---

## Puntos a Mejorar

### Arquitectura y Código
1. **Variables globales** `FILAS`, `COLUMNAS`, `memoria` en `main.cpp` (namespace anónimo) - dificultan testing y concurrencia
2. **Acoplamiento fuerte** - `Orden.h` define `inline int FILAS/COLUMNAS` que usan todos los módulos
3. **Parser JSON casero** - Frágil, no maneja escaped chars completos, arrays anidados complejos
4. **Sin tests automatizados** - Validación manual únicamente
5. **Manejo de errores** - `std::string& mensaje` por referencia en lugar de `std::expected`/`std::optional` (C++17)
6. **Recursión en `pedirEntero()`** - Riesgo de stack overflow en entrada inválida persistente

### Funcionalidad
7. **Sin deshacer/rehacer** - Correcciones son destructivas
8. **Sin importación masiva** - Solo un archivo a la vez
9. **Visualización HTML básica** - Sin zoom, pan, tooltips ricos, leyendas interactivas
10. **Grid fijo en HTML** - `7px` celdas, no responsive
11. **Validación parcial en creación** - Solo al final, no en tiempo real

### Robustez
12. **Path traversal** - `rutaEnAtaques()` mitiga pero no usa `std::filesystem::weakly_canonical`
13. **Race condition** - `listarArchivosAtaques()` crea directorio si no existe (TOCTOU)
14. **Sin validación de límites** en `combinarOrden()` para `espera` (overflow `unsigned int`)
15. **Encoding** - Asume UTF-8, no maneja BOM en CSV

### UX/Consola
16. **Sin colores** - Salida monocromática
17. **Sin autocompletado** - Entrada manual de coordenadas
18. **Paginado fijo** (20 líneas) - No configurable
19. **Mensajes en español hardcodeado** - Sin i18n

---

## Qué No Funciona / Limitaciones Conocidas

| Problema | Detalle | Impacto |
|----------|---------|---------|
| **Parser JSON frágil** | `extraerValorJson()`/`extraerArrayJson()` fallan con: strings con `:` o `,`, escapes `\uXXXX`, números científicos, `null`, booleanos `true`/`false` sin comillas | Archivos JSON válidos estándar pueden no leerse |
| **CSV estricto** | Requiere cabecera exacta, sin espacios extra, sin comillas, 10 columnas fijas | Archivos exportados de Excel/Calc a menudo fallan |
| **Grid global mutable** | `FILAS`/`COLUMNAS` cambian al leer JSON con `tamano` distinto | Cargar archivo A (20x20) → Cargar archivo B (10x10) rompe validación de A en memoria |
| **Validación de siguiente** | En `pedirOrden()` rechaza `(0,0)` como "sin movimiento" pero `(0,0)` es coordenada válida | No se puede mover a esquina superior izquierda |
| **Combinar órdenes** | `combinarOrden()` suma `espera` pero no detecta conflictos semánticos (ej: despegue + aterrizaje en misma celda) | Estados inválidos silenciosos |
| **Memoria no persistente** | Al salir se pierde todo; no hay "guardar automático" | Pérdida de trabajo si no usa opción 6 |
| **Sin validación cruzada CSV↔JSON** | Guardar como JSON pierde formato CSV original | No round-trip perfecto |
| **HTML estático** | Sin JS, sin interactividad, tooltip solo `title` nativo | Limitado para análisis visual profundo |
| **Límite 1000x1000** | Hardcodeado en `configurarGrilla()` y validación JSON | Grillas mayores fallan |
| **Unicode en consola** | `system("cls")`/`clear` y salida ASCII; símbolos `v` `^` `>` `<` pueden verse mal en algunas codepages | Display roto en Windows sin UTF-8 |

---

## Compilación y Ejecución

```bash
# CodeBlocks
# Abrir fede.cbp → Build (F9)

# Línea de comandos (MinGW)
g++ -std=c++17 -Iinclude -Wall -g src/*.cpp -o bin/fede

# Ejecutar (desde raíz del proyecto para que encuentre carpeta ataques/)
./bin/fede
# o desde bin/Debug/
./fede.exe
```

**Requisitos:** C++17, `<filesystem>` (GCC 8+ / MSVC 19.14+)

---

## Ejemplos de Archivos Válidos

**JSON (`ataques/ejemplo.json`):**
```json
{
  "filas": 10,
  "columnas": 10,
  "registros": [
    {"x":0,"y":0,"despegue":true,"siguiente_x":1,"siguiente_y":0},
    {"x":1,"y":0,"soltarGranada1":true,"siguiente_x":2,"siguiente_y":0},
    {"x":2,"y":0,"aterrizaje":true}
  ]
}
```

**CSV (`ataques/ejemplo.dat`):**
```csv
x,y,espera,bomba1,bomba2,kamikaze,aterrizaje,despegue,siguiente_x,siguiente_y
0,0,0,0,0,0,0,1,1,0
1,0,0,1,0,0,0,0,2,0
2,0,0,0,0,0,1,0,0,0
```

---

## Dependencias Externas

- **Ninguna** - Solo STL (C++17): `<filesystem>`, `<vector>`, `<string>`, `<fstream>`, `<sstream>`, `<iostream>`, `<iomanip>`, `<cmath>`, `<cctype>`

---

*Generado automáticamente tras análisis del código fuente - Oct 2026*