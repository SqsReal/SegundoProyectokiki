#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<vector>
#include<cmath>
#include<algorithm>
#include<string>
#include<sstream>
#include<iomanip>
#include<cstdio>
#include <cstring>
#include <cctype>

// Dear ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


//como vamo?
// VARIABLES GLOBALES
// 

// Estructura para almacenar un punto
struct Punto
{
    int x;
    int y;
};


// Tipos de figuras
enum TipoFigura
{
    LINEA,
    CIRCULO,
    TRIANGULO,
    CUADRADO
};


// Estructura que almacena cada figura individualmente
struct Figura
{
    TipoFigura tipo = LINEA;
    std::vector<Punto> pixeles;

    int minX = 0;
    int maxX = 0;
    int minY = 0;
    int maxY = 0;

    ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
};


// Vector que contiene todas las figuras
std::vector<Figura> figuras;


// Radio inicial de los circulos
const int radio = 50;


// Color inicial de las figuras
ImVec4 colorActual = ImVec4(0.72f, 0.53f, 0.04f, 1.0f);


// Variables para controlar los modos
TipoFigura modoActual = CIRCULO;

bool modoColor = false;
bool modoTransformar = false;


// Variables para dibujar
bool dibujando = false;

int inicioX = 0;
int inicioY = 0;

int finalX = 0;
int finalY = 0;


// Variables para transformar figuras
int figuraSeleccionada = -1;

bool arrastrando = false;

double anteriorX = 0;
double anteriorY = 0;


// Variables para el color hexadecimal
char codigoColor[16] = "#FF0000";

bool ventanaColor = false;


// Dimensiones de la ventana
int anchoVentana = 1100;
int altoVentana = 800;


// Espacio reservado para los botones laterales
const float anchoPanel = 85.0f;



// FUNCIONES AUXILIARES
// 


// Agrega un punto a una figura
void agregarPunto(int x, int y, std::vector<Punto>& pixeles)
{
    pixeles.push_back({ x, y });
}


// Actualiza los limites de una figura
void actualizarLimites(Figura& figura)
{
    if (figura.pixeles.empty())
        return;

    figura.minX = figura.maxX = figura.pixeles[0].x;
    figura.minY = figura.maxY = figura.pixeles[0].y;

    for (const Punto& p : figura.pixeles)
    {
        figura.minX = std::min(figura.minX, p.x);
        figura.maxX = std::max(figura.maxX, p.x);

        figura.minY = std::min(figura.minY, p.y);
        figura.maxY = std::max(figura.maxY, p.y);
    }
}



// ALGORITMO DE BRESENHAM PARA LINEAS
// 

void Bresenham(int x0, int y0, int x1, int y1,
    std::vector<Punto>& pixeles)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int error = dx - dy;

    while (true)
    {
        agregarPunto(x0, y0, pixeles);

        if (x0 == x1 && y0 == y1)
            break;

        int error2 = 2 * error;

        if (error2 > -dy)
        {
            error -= dy;
            x0 += sx;
        }

        if (error2 < dx)
        {
            error += dx;
            y0 += sy;
        }
    }
}



// ALGORITMO DE PUNTO MEDIO PARA CIRCULOS
//

void PuntoMedioCirculo(int xc, int yc, int r,
    std::vector<Punto>& pixeles)
{
    int x = 0;
    int y = r;

    int p = 1 - r;

    while (x <= y)
    {
        // Aprovechamos la simetria del circulo
        // para obtener los 8 puntos

        agregarPunto(xc + x, yc + y, pixeles);
        agregarPunto(xc - x, yc + y, pixeles);

        agregarPunto(xc + x, yc - y, pixeles);
        agregarPunto(xc - x, yc - y, pixeles);

        agregarPunto(xc + y, yc + x, pixeles);
        agregarPunto(xc - y, yc + x, pixeles);

        agregarPunto(xc + y, yc - x, pixeles);
        agregarPunto(xc - y, yc - x, pixeles);


        // Algoritmo de punto medio

        if (p < 0)
        {
            p = p + 2 * x + 3;
        }
        else
        {
            p = p + 2 * (x - y) + 5;

            y--;
        }

        x++;
    }
}



// FUNCION PARA RELLENAR POLIGONOS
//
void rellenarPoligono(const std::vector<Punto>& vertices,std::vector<Punto>& pixeles)
{
    if (vertices.size() < 3)
        return;

    int minY = vertices[0].y;
    int maxY = vertices[0].y;

    for (const Punto& p : vertices)
    {
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    // Recorremos cada fila del poligono

    for (int y = minY; y <= maxY; y++)
    {
        std::vector<double> intersecciones;

        for (size_t i = 0; i < vertices.size(); i++)
        {
            Punto a = vertices[i];
            Punto b = vertices[(i + 1) % vertices.size()];

            if (a.y == b.y)
                continue;

            if (y >= std::min(a.y, b.y) &&
                y < std::max(a.y, b.y))
            {
                double x = a.x +
                    (double)(y - a.y) *
                    (b.x - a.x) / (b.y - a.y);

                intersecciones.push_back(x);
            }
        }

        std::sort(intersecciones.begin(), intersecciones.end());

        for (size_t i = 0; i + 1 < intersecciones.size(); i += 2)
        {
            int inicio = (int)std::ceil(intersecciones[i]);
            int fin = (int)std::floor(intersecciones[i + 1]);

            for (int x = inicio; x <= fin; x++)
            {
                agregarPunto(x, y, pixeles);
            }
        }
    }
}



// FUNCION PARA GENERAR UNA FIGURA
// 
Figura generarFigura(TipoFigura tipo,int x0,int y0,int x1,int y1)
{
    Figura figura;

    figura.tipo = tipo;
    figura.color = colorActual;

    int minX = std::min(x0, x1);
    int maxX = std::max(x0, x1);

    int minY = std::min(y0, y1);
    int maxY = std::max(y0, y1);


    // LINEA BRESENHAM

    if (tipo == LINEA)
    {
        Bresenham(x0, y0, x1, y1, figura.pixeles);
    }


    // CIRCULO DE PUNTO MEDIO

    else if (tipo == CIRCULO)
    {
        int dx = x1 - x0;
        int dy = y1 - y0;

        int r = (int)std::sqrt(dx * dx + dy * dy);

        PuntoMedioCirculo(x0, y0, r, figura.pixeles);

        // Relleno interior del circulo

        for (int y = -r; y <= r; y++)
        {
            for (int x = -r; x <= r; x++)
            {
                if (x * x + y * y <= r * r)
                {
                    agregarPunto(x0 + x, y0 + y, figura.pixeles);
                }
            }
        }
    }


    // TRIANGULO

    else if (tipo == TRIANGULO)
    {
        std::vector<Punto> vertices =
        {
            { (x0 + x1) / 2, minY },
            { minX, maxY },
            { maxX, maxY }
        };

        rellenarPoligono(vertices, figura.pixeles);

        for (int i = 0; i < 3; i++)
        {
            Punto a = vertices[i];
            Punto b = vertices[(i + 1) % 3];

            Bresenham(a.x, a.y, b.x, b.y, figura.pixeles);
        }
    }


    // CUADRADO

    else if (tipo == CUADRADO)
    {
        std::vector<Punto> vertices =
        {
            { minX, minY },
            { maxX, minY },
            { maxX, maxY },
            { minX, maxY }
        };

        rellenarPoligono(vertices, figura.pixeles);

        for (int i = 0; i < 4; i++)
        {
            Punto a = vertices[i];
            Punto b = vertices[(i + 1) % 4];

            Bresenham(a.x, a.y, b.x, b.y, figura.pixeles);
        }
    }




    actualizarLimites(figura);

    return figura;
}
//
// FUNCION PARA DETECTAR UNA FIGURA
// 
int seleccionarFigura(int x, int y)
{
    // Revisamos las figuras desde la ultima creada

    for (int i = (int)figuras.size() - 1; i >= 0; i--)
    {
        Figura& figura = figuras[i];

        int margen = 5;

        if (x >= figura.minX - margen &&
            x <= figura.maxX + margen &&
            y >= figura.minY - margen &&
            y <= figura.maxY + margen)
        {
            return i;
        }
    }

    return -1;
}



// FUNCION PARA TRANSFORMAR FIGURAS
// 

void moverFigura(int indice, int dx, int dy)
{
    if (indice < 0 || indice >= (int)figuras.size())
        return;

    Figura& figura = figuras[indice];

    // Las lineas tampoco se transforman

    if (figura.tipo == LINEA)
        return;

    for (Punto& p : figura.pixeles)
    {
        p.x += dx;
        p.y += dy;
    }

    actualizarLimites(figura);
}



// FUNCION PARA CONVERTIR HEXADECIMAL A RGB
// 

bool convertirColor(const char* codigo, ImVec4& color)
{
    unsigned int r, g, b;

    const char* texto = codigo;

    if (texto[0] == '#')
        texto++;

    if (std::strlen(texto) != 6)
        return false;

    // Verificamos que todos los caracteres sean hexadecimales
    for (int i = 0; i < 6; i++)
    {
        if (!std::isxdigit(static_cast<unsigned char>(texto[i])))
            return false;
    }

    // Convertimos el hexadecimal a RGB
    if (sscanf_s(texto, "%2x%2x%2x", &r, &g, &b) != 3)
        return false;

    color = ImVec4(r / 255.0f,g / 255.0f,b / 255.0f,1.0f);

    return true;
}



// FUNCION PARA APLICAR COLOR A UNA FIGURA
//

void aplicarColor(int indice)
{
    if (indice < 0 || indice >= (int)figuras.size())
        return;

    // No permitimos rellenar las lineas

    if (figuras[indice].tipo == LINEA)
        return;

    figuras[indice].color = colorActual;
}



// FUNCION PARA OBTENER EL NOMBRE DEL MODO
// 

const char* obtenerNombreModo()
{
    switch (modoActual)
    {
    case LINEA:
        return "Linea Bresenham";

    case CIRCULO:
        return "Circulo";

    case TRIANGULO:
        return "Triangulo";

    case CUADRADO:
        return "Cuadrado";
    }

    return "Desconocido";
}



// FUNCION PARA ACTUALIZAR EL MODO
// 

void cambiarModo()
{
    modoActual = (TipoFigura)((modoActual + 1) % 4);

    std::cout << "Modo actual: "
        << obtenerNombreModo() << std::endl;
}



// FUNCION PARA DIBUJAR EL FONDO DE CUADROS
// 

void dibujarFondo(
    ImDrawList* drawList,
    ImVec2 inicio,
    ImVec2 fin)
{
    const float tamano = 25.0f;

    int columnas = (int)((fin.x - inicio.x) / tamano) + 1;
    int filas = (int)((fin.y - inicio.y) / tamano) + 1;

    for (int fila = 0; fila < filas; fila++)
    {
        for (int columna = 0; columna < columnas; columna++)
        {
            float x = inicio.x + columna * tamano;
            float y = inicio.y + fila * tamano;

            ImU32 color;

            if ((fila + columna) % 2 == 0)
                color = IM_COL32(255, 255, 255, 255);
            else
                color = IM_COL32(20, 20, 20, 255);

            drawList->AddRectFilled(
                ImVec2(x, y),
                ImVec2(x + tamano, y + tamano),
                color
            );
        }
    }
}



// FUNCION PARA DIBUJAR LAS FIGURAS
// 

void dibujarFiguras(
    ImDrawList* drawList,
    ImVec2 inicio)
{
    for (const Figura& figura : figuras)
    {
        ImU32 color = ImGui::ColorConvertFloat4ToU32(
            figura.color
        );

        for (const Punto& p : figura.pixeles)
        {
            float x = inicio.x + p.x;
            float y = inicio.y + p.y;

            drawList->AddRectFilled(
                ImVec2(x, y),
                ImVec2(x + 2.0f, y + 2.0f),
                color
            );
        }
    }
}



// FUNCION QUE DETECTA EL TAMANO DE LA VENTANA
// 

#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
 
// Ahora cada vertice lleva posicion (x, y, z) y color (r, g, b),
// por eso el vertex shader recibe dos atributos y le pasa el color al fragment shader
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 vColor;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"    vColor = aColor;\n"
"}\0";
 
const char* fragmentShaderSource = "#version 330 core\n"
"in vec3 vColor;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(vColor, 1.0f);\n"
"}\n";
 
 
// Vector que contiene los vertices de las casitas (posicion + color)
std::vector<float> vertices;
 
 
// VBO Y VAO
GLuint VBO, VAO;
 
 
// Tamano de la casita (en pixeles)
const int mitadAncho = 40;   // el cuerpo mide 80 de ancho
const int mitadAlto = 30;    // el cuerpo mide 60 de alto
const int alturaTecho = 40;  // que tan arriba queda la punta del techo
 
 
// Color actual con el que se esta dibujando la casita
float colorR = 1.0f, colorG = 1.0f, colorB = 1.0f;
 
 
// Funcion para agregar un punto a nuestro vector
void agregarPunto(int x, int y, std::vector<float>& vertices)
{
	// Convertimos el pixel a coordenadas de OpenGL
 
	float xOpenGL = (2.0f * x / 799.0f) - 1.0f;
 
	float yOpenGL = 1.0f - (2.0f * y / 799.0f);
 
 
	// Posicion
	vertices.push_back(xOpenGL);
	vertices.push_back(yOpenGL);
	vertices.push_back(0.0f);
 
	// Color
	vertices.push_back(colorR);
	vertices.push_back(colorG);
	vertices.push_back(colorB);
}
 
 
// Algoritmo de Bresenham (punto medio para rectas) para dibujar una linea
void LineaBresenham(int x0, int y0, int x1, int y1, std::vector<float>& vertices)
{
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
 
	// Direccion en la que avanzamos en cada eje
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
 
	int err = dx - dy;
 
	while (true)
	{
		agregarPunto(x0, y0, vertices);
 
		if (x0 == x1 && y0 == y1)
		{
			break;
		}
 
		int e2 = 2 * err;
 
		if (e2 > -dy)
		{
			err = err - dy;
			x0 = x0 + sx;
		}
 
		if (e2 < dx)
		{
			err = err + dx;
			y0 = y0 + sy;
		}
	}
}
 
 
// Dibuja un rectangulo uniendo 4 lineas
void Rectangulo(int x0, int y0, int x1, int y1, std::vector<float>& vertices)
{
	LineaBresenham(x0, y0, x1, y0, vertices);
	LineaBresenham(x1, y0, x1, y1, vertices);
	LineaBresenham(x1, y1, x0, y1, vertices);
	LineaBresenham(x0, y1, x0, y0, vertices);
}
 
 
// Dibuja una casita completa con centro en (xc, yc)
void DibujarCasa(int xc, int yc, std::vector<float>& vertices)
{
	int izquierda = xc - mitadAncho;
	int derecha = xc + mitadAncho;
	int arriba = yc - mitadAlto;
	int abajo = yc + mitadAlto;
 
	// Cuerpo de la casa
	Rectangulo(izquierda, arriba, derecha, abajo, vertices);
 
	// Techo (triangulo): sobresale un poquito de las paredes
	int puntaX = xc;
	int puntaY = arriba - alturaTecho;
 
	LineaBresenham(izquierda - 10, arriba, puntaX, puntaY, vertices);
	LineaBresenham(puntaX, puntaY, derecha + 10, arriba, vertices);
	LineaBresenham(derecha + 10, arriba, izquierda - 10, arriba, vertices);
 
	// Puerta
	Rectangulo(xc - 30, yc, xc - 10, abajo, vertices);
 
	// Ventana
	Rectangulo(xc + 10, yc - 15, xc + 30, yc + 5, vertices);
}
 
 
// Esta funcion actualiza el VBO cada vez que agregamos una nueva casa
void actualizarVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
 
	glBufferData(
		GL_ARRAY_BUFFER,
		vertices.size() * sizeof(float),
		vertices.data(),
		GL_DYNAMIC_DRAW
	);
 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
 
 
// Funcion que detecta cuando hacemos click con el mouse
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double xpos, ypos;
 
		// Obtenemos la posicion del mouse
		glfwGetCursorPos(window, &xpos, &ypos);
 
		int x = (int)xpos;
		int y = (int)ypos;
 
 
		// Cada click genera un color nuevo (entre 0.3 y 1.0 para que se vea sobre el fondo)
		colorR = 0.3f + 0.7f * (rand() / (float)RAND_MAX);
		colorG = 0.3f + 0.7f * (rand() / (float)RAND_MAX);
		colorB = 0.3f + 0.7f * (rand() / (float)RAND_MAX);
 
 
		// Cada click genera una nueva casita
 
		DibujarCasa(
			x,
			y,
			vertices
		);
 
 
		// Actualizamos el VBO para que OpenGL
		// conozca los nuevos vertices
 
		actualizarVBO();
 
 
		std::cout << "Casa creada en: ("
			<< x << ", " << y << ")" << std::endl;
	}
}
 
 
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}



// FUNCION PRINCIPAL
// 

int main()
{
    glfwInit();

    // Le dice a GLFW que queremos usar la version 3.3
    // de OpenGL y que queremos el perfil core

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(
        1100,
        800,
        "Algoritmos de Rasterizacion",
        NULL,
        NULL
    );


    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;

        glfwTerminate();

        return -1;
    }


    // Introduce la ventana como contexto actual de OpenGL

    glfwMakeContextCurrent(window);


    // Inicializamos GLAD

    if (!gladLoadGL())
    {
        std::cout << "Failed to initialize GLAD" << std::endl;

        glfwTerminate();

        return -1;
    }


    // Configuracion del viewport

    glfwSetFramebufferSizeCallback(
        window,
        framebuffer_size_callback
    );


    glViewport(0, 0, 1100, 800);


    
    // INICIALIZACION DE DEAR IMGUI
    // 
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;


    // Estilo de ImGui

    ImGui::StyleColorsDark();

    ImGuiStyle& estilo = ImGui::GetStyle();

    estilo.WindowRounding = 5.0f;
    estilo.FrameRounding = 5.0f;
    estilo.WindowBorderSize = 0.0f;


    // Inicializamos los backends

    ImGui_ImplGlfw_InitForOpenGL(window, true);

    ImGui_ImplOpenGL3_Init("#version 330 core");


    
    // VARIABLES PARA EL MOUSE
    // 

    bool teclaQAnterior = false;
    bool teclaWAnterior = false;

    bool tecla1Anterior = false;
    bool tecla2Anterior = false;
    bool tecla3Anterior = false;
    bool tecla4Anterior = false;


    
    // CICLO PRINCIPAL
    // 

    while (!glfwWindowShouldClose(window))
    {
        // Procesamos los eventos

        glfwPollEvents();


        // Obtenemos el tamano de la ventana

        glfwGetWindowSize(
            window,
            &anchoVentana,
            &altoVentana
        );


        // Iniciamos un nuevo frame de ImGui

        ImGui_ImplOpenGL3_NewFrame();

        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();


        ImGuiIO& ioActual = ImGui::GetIO();


        
        // AREA DE DIBUJO
        // 
        ImVec2 inicioCanvas(
            anchoPanel,
            0.0f
        );

        ImVec2 finCanvas(
            anchoVentana - anchoPanel,
            (float)altoVentana
        );


        ImDrawList* drawList = ImGui::GetBackgroundDrawList();


        // Dibujamos el fondo de cuadros

        dibujarFondo(
            drawList,
            inicioCanvas,
            finCanvas
        );


        // Dibujamos las figuras existentes

        dibujarFiguras(
            drawList,
            inicioCanvas
        );


        
        // PANEL IZQUIERDO: TRANSFORMAR
        // 

        ImGui::SetNextWindowPos(
            ImVec2(0, 10),
            ImGuiCond_Always
        );

        ImGui::SetNextWindowSize(
            ImVec2(anchoPanel, 75),
            ImGuiCond_Always
        );

        ImGui::Begin(
            "PanelTransformar",
            NULL,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings
        );


        // Boton rojo para transformar

        ImGui::PushStyleColor(
            ImGuiCol_Button,
            ImVec4(0.85f, 0.05f, 0.05f, 1.0f)
        );

        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered,
            ImVec4(1.0f, 0.15f, 0.15f, 1.0f)
        );

        ImGui::PushStyleColor(
            ImGuiCol_Text,
            ImVec4(1, 1, 1, 1)
        );


        if (ImGui::Button("Mover", ImVec2(75, 50)))
        {
            modoTransformar = !modoTransformar;

            modoColor = false;

            figuraSeleccionada = -1;
            arrastrando = false;
        }


        ImGui::PopStyleColor(3);


        ImGui::End();


        
        // PANEL DERECHO: COLOR
        // 

        ImGui::SetNextWindowPos(
            ImVec2(anchoVentana - anchoPanel, 10),
            ImGuiCond_Always
        );

        ImGui::SetNextWindowSize(
            ImVec2(anchoPanel, 75),
            ImGuiCond_Always
        );


        ImGui::Begin(
            "PanelColor",
            NULL,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings
        );


        // Boton azul para cambiar el color

        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.0f, 0.25f, 0.75f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0.1f, 0.4f, 1.0f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1, 1, 1, 1));


        if (ImGui::Button("Color", ImVec2(75, 50)))
        {
            ventanaColor = true;

            ImGui::OpenPopup("CambiarColor");
        }


        ImGui::PopStyleColor(3);


        // Ventana emergente del color

        if (ImGui::BeginPopupModal(
            "CambiarColor",
            &ventanaColor,
            ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Digite codigo hexadecimal del color");

            ImGui::SetNextItemWidth(180);

            bool enter = ImGui::InputText(
                "##Codigo",
                codigoColor,
                sizeof(codigoColor),
                ImGuiInputTextFlags_EnterReturnsTrue
            );


            if (enter)
            {
                ImVec4 nuevoColor;

                if (convertirColor(codigoColor, nuevoColor))
                {
                    colorActual = nuevoColor;

                    modoColor = true;
                    modoTransformar = false;

                    ventanaColor = false;

                    std::cout << "Color hexadecimal aplicado: "
                        << codigoColor << std::endl;

                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    std::cout << "Codigo hexadecimal invalido" << std::endl;
                }
            }


            ImGui::Text("Ejemplo: #FF0000");

            if (ImGui::Button("Cancelar"))
            {
                ventanaColor = false;

                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }


        ImGui::End();


        
        // INFORMACION DEL MODO ACTUAL
        //
        ImGui::SetNextWindowPos(ImVec2(anchoPanel + 10, 10),ImGuiCond_Always
        );

        ImGui::SetNextWindowSize(
            ImVec2(230, 75),
            ImGuiCond_Always
        );

        ImGui::Begin(
            "Informacion",
            NULL,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoSavedSettings
        );

        ImGui::Text("Figura: %s", obtenerNombreModo());

        if (modoColor)
            ImGui::TextColored(
                ImVec4(0.3f, 1.0f, 0.3f, 1.0f),
                "Modo: COLOR"
            );

        else if (modoTransformar)
            ImGui::TextColored(
                ImVec4(1.0f, 0.7f, 0.2f, 1.0f),
                "Modo: MOVER"
            );

        else
            ImGui::Text("Modo: DIBUJO");


        ImGui::End();


        
        // DETECCION DE TECLAS
        // 

        bool teclaQ = glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS;
        bool teclaW = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;

        bool tecla1 = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
        bool tecla2 = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
        bool tecla3 = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
        bool tecla4 = glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;



        if (!ioActual.WantTextInput)
        {
            if (teclaQ && !teclaQAnterior)
                cambiarModo();

            if (tecla1 && !tecla1Anterior)
                modoActual = LINEA;

            if (tecla2 && !tecla2Anterior)
                modoActual = CIRCULO;

            if (tecla3 && !tecla3Anterior)
                modoActual = TRIANGULO;

            if (tecla4 && !tecla4Anterior)
                modoActual = CUADRADO;



            // W permite salir de los modos especiales

            if (teclaW && !teclaWAnterior)
            {
                modoColor = false;
                modoTransformar = false;

                figuraSeleccionada = -1;
                arrastrando = false;

                std::cout << "Modo especial desactivado" << std::endl;
            }
        }


        teclaQAnterior = teclaQ;
        teclaWAnterior = teclaW;

        tecla1Anterior = tecla1;
        tecla2Anterior = tecla2;
        tecla3Anterior = tecla3;
        tecla4Anterior = tecla4;


       
        // DETECCION DEL MOUSE
        // 

        double mouseX, mouseY;

        glfwGetCursorPos(
            window,
            &mouseX,
            &mouseY
        );


        // Convertimos la posicion del mouse
        // a coordenadas relativas al canvas

        int x = (int)(mouseX - inicioCanvas.x);
        int y = (int)(mouseY - inicioCanvas.y);


        // Verificamos que el mouse este dentro del canvas

        bool dentroCanvas =
            mouseX >= inicioCanvas.x &&
            mouseX <= finCanvas.x &&
            mouseY >= inicioCanvas.y &&
            mouseY <= finCanvas.y;


        // Evitamos que los botones de ImGui
        // generen figuras accidentalmente

        bool mouseLibre = !ioActual.WantCaptureMouse;


        
        // MODO TRANSFORMAR
        // 

        if (modoTransformar)
        {
            if (dentroCanvas && mouseLibre &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                figuraSeleccionada = seleccionarFigura(x, y);

                if (figuraSeleccionada != -1 &&
                    figuras[figuraSeleccionada].tipo != LINEA)
                {
                    arrastrando = true;

                    anteriorX = mouseX;
                    anteriorY = mouseY;
                }
                else
                {
                    arrastrando = false;
                }
            }


            if (arrastrando &&
                ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                int dx = (int)(mouseX - anteriorX);
                int dy = (int)(mouseY - anteriorY);

                if (dx != 0 || dy != 0)
                {
                    moverFigura(figuraSeleccionada, dx, dy);

                    anteriorX = mouseX;
                    anteriorY = mouseY;
                }
            }


            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                arrastrando = false;
            }
        }


        
        // MODO COLOR
        // 

        else if (modoColor)
        {
            if (dentroCanvas && mouseLibre &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                int indice = seleccionarFigura(x, y);

                if (indice != -1)
                {
                    aplicarColor(indice);

                    std::cout << "Color aplicado a la figura"
                        << std::endl;
                }
            }
        }


        
        // MODO DIBUJO
        // 
        else
        {
            // Detectamos el primer click

            if (dentroCanvas && mouseLibre &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                dibujando = true;

                inicioX = x;
                inicioY = y;
            }


            // Detectamos cuando soltamos el mouse

            if (dibujando &&
                ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                finalX = x;
                finalY = y;

                // Creamos la figura correspondiente

                Figura nuevaFigura = generarFigura(modoActual,inicioX,inicioY,finalX,finalY);


                if (!nuevaFigura.pixeles.empty())
                {
                    figuras.push_back(nuevaFigura);

                    std::cout << "Figura creada: "
                        << obtenerNombreModo()
                        << std::endl;
                }

                dibujando = false;
            }
        }


        
        // DIBUJAMOS LA FIGURA EN TIEMPO REAL
        // 
        if (dibujando &&
            !modoColor &&
            !modoTransformar)
        {
            int actualX = x;
            int actualY = y;

            Figura temporal = generarFigura(
                modoActual,
                inicioX,
                inicioY,
                actualX,
                actualY
            );

            ImU32 colorTemporal =
                ImGui::ColorConvertFloat4ToU32(colorActual);

            for (const Punto& p : temporal.pixeles)
            {
                drawList->AddRectFilled(
                    ImVec2(
                        inicioCanvas.x + p.x,
                        inicioCanvas.y + p.y
                    ),
                    ImVec2(
                        inicioCanvas.x + p.x + 2,
                        inicioCanvas.y + p.y + 2
                    ),
                    colorTemporal
                );
            }
        }


        
        // RENDERIZADO
        // 

        ImGui::Render();


        glClearColor(0.1f,0.1f,0.1f,1.0f);

        glClear(GL_COLOR_BUFFER_BIT);


        // Renderizamos todos los elementos de Dear ImGui

        ImGui_ImplOpenGL3_RenderDrawData(
            ImGui::GetDrawData()
        );


        glfwSwapBuffers(window);
    }


    
    // LIBERACION DE RECURSOS
    // 

    ImGui_ImplOpenGL3_Shutdown();

    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();


    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
 
 
int main()
{
	// Semilla para que los colores aleatorios cambien en cada ejecucion
	srand((unsigned int)time(NULL));
 
	glfwInit();
 
	//le dice al glfw que queremos usar la version 3.3 de opengl y que queremos usar el perfil core
 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
 
	GLFWwindow* window = glfwCreateWindow(800, 800, "Casitas", NULL, NULL);
 
 
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
 
		glfwTerminate();
 
		return -1;
	}
 
 
	// Funcion que detecta los clicks del mouse
 
	glfwSetMouseButtonCallback(
		window,
		mouse_button_callback
	);
 
 
	//introduce la ventana que creamos como contexto actual de opengl, es decir, que todo lo que hagamos a partir de ahora se va a dibujar en esa ventana
 
	glfwMakeContextCurrent(window);
 
 
	gladLoadGL();
 
 
	//esto indica de donde a donde queremos que open gl se renderize
 
	glViewport(0, 0, 800, 800);
 
 
	//shaders
 
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
 
	glShaderSource(
		vertexShader,
		1,
		&vertexShaderSource,
		NULL
	);
 
	glCompileShader(vertexShader);
 
 
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
 
	glShaderSource(
		fragmentShader,
		1,
		&fragmentShaderSource,
		NULL
	);
 
	glCompileShader(fragmentShader);
 
 
	//el shader program es un programa que contiene los shaders que vamos a usar para dibujar, en este caso el vertex shader y el fragment shader
 
	GLuint shaderProgram = glCreateProgram();
 
	glAttachShader(shaderProgram, vertexShader);
 
	glAttachShader(shaderProgram, fragmentShader);
 
 
	//El linking es el proceso de unir los shaders en un programa que pueda ser ejecutado por la GPU
 
	glLinkProgram(shaderProgram);
 
 
	glDeleteShader(vertexShader);
 
	glDeleteShader(fragmentShader);
 
 
	//El VBO Y VBA
 
	//el VBO es un objeto que contiene los datos de los vertices
 
	//el VAO es un objeto que contiene la configuracion de los atributos de los vertices
 
 
	glGenBuffers(1, &VBO);
 
	glGenVertexArrays(1, &VAO);
 
 
	glBindVertexArray(VAO);
 
 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
 
 
	//Ahora utilizamos GL_DYNAMIC_DRAW porque los vertices
	//van a cambiar cada vez que hagamos click
 
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
 
 
	// Atributo 0: posicion (3 floats), cada vertice ahora ocupa 6 floats
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
 
	glEnableVertexAttribArray(0);
 
 
	// Atributo 1: color (3 floats), empieza despues de los 3 floats de la posicion
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
 
	glEnableVertexAttribArray(1);
 
 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
 
	glBindVertexArray(0);
 
 
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.02f, 0.3f, 0.3f, 1.0f);
 
		glClear(GL_COLOR_BUFFER_BIT);
 
 
		glUseProgram(shaderProgram);
 
 
		glBindVertexArray(VAO);
 
 
		// Dibujamos los puntos generados
		// por el algoritmo de Bresenham
 
		glPointSize(2.0f);
 
 
		// Ahora son 6 floats por vertice, no 3
		glDrawArrays(
			GL_POINTS,
			0,
			vertices.size() / 6
		);
 
 
		glfwSwapBuffers(window);
 
 
		// funcion que permite que procese todos los eventos extraidos
 
		glfwPollEvents();
	}
 
 
	glDeleteVertexArrays(1, &VAO);
 
	glDeleteBuffers(1, &VBO);
 
	glDeleteProgram(shaderProgram);
 
	glfwDestroyWindow(window);
 
	glfwTerminate();
 
	return 0;
}
