# TP Grupal AyED: sistema de guía de apoyo para drones

UTN FRBA · Algoritmos y Estructuras de Datos · Curso K1051 · Docente: Ing. Mendez

Fecha de entrega: 5/11/2026

Programa en C++ que simula la lógica offline de un dron cuando el enemigo bloquea el GPS. El dron sigue una
ruta cargada en una tabla de 200 x 200 casilleros. Cada casillero puede tener una orden (despegar, aterrizar,
soltar granadas, ataque kamikaze, esperar y moverse a la celda siguiente).

## Integrantes

Curso: K1051

| Apellido y nombre | Legajo | Correo institucional (FRBA) | Usuario de GitHub | Parte del trabajo |
| --- | --- | --- | --- | --- |
| Fernandez Federico | 235.036-7 | fedfernandez@frba.utn.edu.ar | fedfernandez | Operaciones 3, 6 y 7 |
| Ocello Santiago | 222.556-6 | socello@frba.utn.edu.ar | SantiOcello | Operaciones 4 y 5 |
| Cisneros Victoria | 222.224-3 | vcisneros@frba.utn.edu.ar | Kaitonline | Estructuras de datos, Operaciones 1 y 2 |

## Estructura del repositorio

```
.
├── README.md
├── TP_AyED_Drones.cbp        Proyecto de Code::Blocks
├── src/
│   └── main.cpp              Código fuente del programa
├── pruebas/                  Archivos binarios de prueba (.dat) y su LEEME
└── informe/
    └── Informe_TP_AyED.pdf   Informe final
```

El ejecutable no se sube al repositorio. Se genera compilando el proyecto (ver más abajo).

## Cómo compilar y ejecutar

**Con Code::Blocks (Windows):**

1. Abrir `TP_AyED_Drones.cbp`.
2. Elegir el target **Debug** y presionar *Build and run* (F9).

**Por consola (MinGW):**

```
g++ -std=c++11 -Wall -Wextra -pedantic -o TP_AyED_Drones.exe src/main.cpp
TP_AyED_Drones.exe
```

El programa usa `conio.h` (`getch()`), por lo que compila en Windows con MinGW.

## Menú del programa

1. Cargar archivo de ataque en memoria (valida todas las reglas de la consigna).
2. Mostrar ataque cargado (en orden de recorrido, con pausa cada 20 líneas).
3. Crear un archivo de ataque nuevo.
4. Corregir un registro del archivo.
5. Corregir un registro en memoria.
6. Guardar memoria en un archivo nuevo.
7. Visualizar el ataque en HTML (tabla de 200 x 200).
8. Salir.

## Cómo probarlo

En la carpeta `pruebas/` hay archivos de ejemplo, válidos e inválidos, con el resultado esperado de cada uno
en `pruebas/LEEME_pruebas.txt`. Desde Code::Blocks, en la opción 1 se escribe, por ejemplo:

```
pruebas/ok_diag.dat
```

## Decisiones principales (el detalle está en el informe)

- Las celdas aledañas son las 8 vecinas, incluidas las diagonales.
- Aterrizar o hacer un ataque kamikaze termina el recorrido.
- Máximo dos granadas: la granada 1 y la granada 2, cada una una sola vez.
- Si un archivo no cumple las reglas, se informa el motivo y no se carga. La memoria que ya estaba cargada no cambia.
- La espera se expresa en segundos.

## Uso de herramientas de IA

Durante el trabajo se usaron herramientas de IA (Gemini y Claude). El detalle de cada una, con el motivo de
uso, la respuesta recibida y los cambios que hicimos, está en la sección 5 del informe (`informe/`).
