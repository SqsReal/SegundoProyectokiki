#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
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
