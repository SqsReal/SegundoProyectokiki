# Motor de Rasterización 2D Interactivo (2D Rasterization Engine)

 Descripción del Proyecto

"Motor de Rasterización 2D Interactivo" es una aplicación desarrollada en C++ utilizando OpenGL, GLFW, GLAD y **Dear ImGui**. El proyecto permite renderizar, manipular y colorear primitivas geométricas bidimensionales (líneas, círculos, triángulos y cuadrados) calculando directamente los píxeles mediante algoritmos clásicos de graficación por computadora en lugar de delegar el rasterizado a la canalización de hardware (*fixed-function pipeline* o *shaders* de fragmentos).

La aplicación incluye un lienzo (*canvas*) dinámico con fondo en cuadrícula, un modo de previsualización en tiempo real al arrastrar el puntero, un inspector de estado visual y un panel flotante para el ajuste de colores en formato hexadecimal.


Tecnologías Utilizadas

* **C++11 / C++14**: Lenguaje de programación base.
* **GLFW**: Creación de la ventana, gestión del contexto de OpenGL y captura de eventos de teclado y ratón.
* **GLAD**: Cargador de funciones para OpenGL 3.3 Core Profile.
* **Dear ImGui**: Renderizado de la interfaz de usuario (menús, paneles interactivos, modales de texto) y dibujado manual de píxeles mediante `ImDrawList`.

---

 Explicación del Código

El código está estructurado en bloques funcionales bien definidos que abarcan desde el cálculo matemático de primitivas hasta el bucle principal de renderizado.

### 1. Estructuras de Datos Principales
* **`Punto`**: Modela las coordenadas de un píxel discreto $(x, y)$ en espacio de pantalla.
* **`Figura`**: Contiene el tipo de primitiva (`LINEA`, `CIRCULO`, `TRIANGULO`, `CUADRADO`), la lista de píxeles que forman la figura (`std::vector<Punto>`), la caja delimitadora o AABB (`minX`, `maxX`, `minY`, `maxY`) para detección de colisiones/selección, y el color asignado (`ImVec4`).

### 2. Algoritmos de Rasterización
* **Algoritmo de Línea de Bresenham (`Bresenham`)**:
  Calcula los píxeles de una línea recta entre dos puntos $(x_0, y_0)$ y $(x_1, y_1)$ utilizando únicamente aritmética entera incremental con una variable de error:
  $$error = \Delta x - \Delta y$$
* **Algoritmo de Punto Medio para Círculos (`PuntoMedioCirculo`)**:
  Genera la circunferencia trazando un octante y aprovechando la simetría octogonal del círculo alrededor del centro $(x_c, y_c)$ con radio $r$. La variable de decisión inicial se define como:
  $$p = 1 - r$$
* **Relleno de Polígonos por Scanline (`rellenarPoligono`)**:
  Implementa un barrido horizontal (*Scanline fill algorithm*). Calcula los puntos de intersección entre las aristas del polígono y cada línea de escaneo horizontal $y$. Posteriormente, ordena las intersecciones de izquierda a derecha y rellena los pares de segmentos interiores.

### 3. Generación y Manipulación de Figuras
* **`generarFigura`**: Recibe las coordenadas iniciales y finales trazadas por el usuario y construye la figura seleccionada combinando los algoritmos de trazado y relleno.
* **`seleccionarFigura`**: Realiza un *Hit-Test* evaluando si la posición actual del cursor cae dentro de la caja delimitadora (AABB) de alguna figura registrada.
* **`moverFigura`**: Traslada la posición de los píxeles pertenecientes a una figura aplicando un desplazamiento $(\Delta x, \Delta y)$.

### 4. Conversión de Color
* **`convertirColor`**: Parsea cadenas de texto con códigos hexadecimales (por ejemplo, `#FF0000` o `FF0000`) y las transforma en un vector flotante RGB normalizado en el rango $[0.0, 1.0]$.
------------------------------------------------------------------------------------------------------------------------------------------------------
## Arquitectura del Sistema

La arquitectura sigue una estructura modular por capas basada en un ciclo de eventos continuo (*Game/Event Loop*).

```
+-------------------------------------------------------------+
|                      Capa de Entrada                        |
|       (GLFW Mouse / Keyboard Input & ImGui Event I/O)       |
+------------------------------+------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|                    Gestor de Modos/Estado                   |
|         [ MODO DIBUJO | MODO MOVER | MODO COLOR ]           |
+------------------------------+------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|                 Motor de Rasterización 2D                   |
|  - Algoritmo de Bresenham                                   |
|  - Algoritmo de Punto Medio (Círculos)                      |
|  - Algoritmo Scanline (Relleno de Polígonos)                |
+------------------------------+------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|                 Capa de Renderizado e Interfaz              |
|  - Fondo en Damero (Canvas Canvas Grid)                     |
|  - ImDrawList Rasterizer (Fuerza Bruta / Cuadros de 2x2px)  |
|  - Paneles de Control e InterfazDear ImGui                  |
+------------------------------+------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|                 OpenGL 3.3 Backend (GLFW/GLAD)              |
+-------------------------------------------------------------+
```

 Componentes de la Arquitectura

1. **Capa de Control de Estado (State Machine)**:
   La aplicación conmuta dinámicamente entre tres estados exclusivos:
   * **DIBUJO**: Captura la posición de clic inicial y liberación de clic del ratón para proyectar y confirmar primitivas.
   * **MOVER (Transformar)**: Permite seleccionar figuras existentes mediante clic y arrastrarlas por la pantalla.
   * **COLOR**: Aplica el color almacenado en `colorActual` a la figura sobre la que se haga clic.

2. **Capa de Renderizado (Software Rasterization over GUI Layer)**:
   En lugar de enviar vértices a la GPU a través de buffers de vértices (VBOs/VAOs), el software calcula los conjuntos de píxeles discretos en memoria CPU (`std::vector<Punto>`). Luego, utiliza la lista de comandos de dibujo de Dear ImGui (`ImDrawList::AddRectFilled`) para dibujar pequeños rectángulos de $2 \times 2$ píxeles en el lienzo.

3. **Capa de Interfaz de Usuario**:
   * Panel izquierdo con botón "Mover" para alternar la transformación.
   * Panel derecho con botón "Color" que despliega un diálogo emergente modal (`ImGui::BeginPopupModal`) para ingresar códigos hexadecimales.
   * Panel superior informativo con el modo activo y figura actual.

---

 Controles e Interacción

  Con La **Q** : Cliclar secuencialmente entre figuras (Línea $\rightarrow$ Círculo $\rightarrow$ Triángulo $\rightarrow$ Cuadrado). 
  Con la **W**: Desactivar modos especiales (Desactiva *Mover* y *Color*, regresando al modo *Dibujo*). 
  Con el  **Clic Izquierdo**: Dibujar figura / Seleccionar figura para mover / Aplicar color (según el modo activo). 
