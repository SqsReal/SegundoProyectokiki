#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <vector>
#include <cmath>
//ñañai
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(184.2/255.0f, 134.0/255.0f, 11.0/255.0f, 1.0f);\n"
//Acuerdate que el color es en RGB 
//cabe aclaear que el rango solo va de 0.0 a 1.0, por lo que si queremos un color mas intenso debemos dividirlo entre 255.0f
"}\n";


// Vector que contiene los vertices de los circulos
std::vector<float> vertices;


// VBO Y VAO
GLuint VBO, VAO;


// Radio de los circulos
const int radio = 50;


// Funcion para agregar un punto a nuestro vector
void agregarPunto(int x, int y, std::vector<float>& vertices)
{
	// Convertimos el pixel a coordenadas de OpenGL

	float xOpenGL = (2.0f * x / 799.0f) - 1.0f;

	float yOpenGL = 1.0f - (2.0f * y / 799.0f);


	vertices.push_back(xOpenGL);
	vertices.push_back(yOpenGL);
	vertices.push_back(0.0f);
}


// Algoritmo de punto medio para dibujar un circulo
void PuntoMedioCirculo(int xc, int yc, int r, std::vector<float>& vertices)
{
	int x = 0;
	int y = r;

	int p = 1 - r;


	while (x <= y)
	{
		// Aprovechamos la simetria del circulo
		// para obtener los 8 puntos

		agregarPunto(xc + x, yc + y, vertices);
		agregarPunto(xc - x, yc + y, vertices);
		agregarPunto(xc + x, yc - y, vertices);
		agregarPunto(xc - x, yc - y, vertices);

		agregarPunto(xc + y, yc + x, vertices);
		agregarPunto(xc - y, yc + x, vertices);
		agregarPunto(xc + y, yc - x, vertices);
		agregarPunto(xc - y, yc - x, vertices);


		// Algoritmo de punto medio

		if (p < 0)
		{
			p = p + 2 * x + 3;
			//se sumo 2 * x + 3 al valor de p, lo que significa que el punto medio esta dentro del circulo 
			// y por lo tanto debemos movernos hacia la derecha, es decir, aumentar x en 1
		}
		else
		{
			p = p + 2 * (x - y) + 5;

			y--;
			//se sumo 2 * (x - y) + 5 al valor de p, lo que significa que el punto medio esta fuera del circulo
		}

		x++;
	}
}


// Esta funcion actualiza el VBO cada vez que agregamos un nuevo circulo
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


		// Cada click genera un nuevo circulo

		PuntoMedioCirculo(
			x,
			y,
			radio,
			vertices
		);


		// Actualizamos el VBO para que OpenGL
		// conozca los nuevos vertices

		actualizarVBO();


		std::cout << "Circulo creado en: ("
			<< x << ", " << y << ")" << std::endl;
	}
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


int main()
{
	glfwInit();

	//le dice al glfw que queremos usar la version 3.3 de opengl y que queremos usar el perfil core

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(800,800,"LearnOpenGL",NULL,NULL);


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

	glBufferData(GL_ARRAY_BUFFER,vertices.size() * sizeof(float),vertices.data(),GL_DYNAMIC_DRAW);


	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3 * sizeof(float),(void*)0);


	glEnableVertexAttribArray(0);


	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);


	//el ultimo numero es la transparencia, 1.0f es opaco y 0.0f es transparente

	glClearColor(55.0f, 0.0f, 0.0f, 1.0f);


	glClear(GL_COLOR_BUFFER_BIT);

	glfwSwapBuffers(window);


	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.02f,0.3f,0.3f,1.0f);

		glClear(GL_COLOR_BUFFER_BIT);


		glUseProgram(shaderProgram);


		glBindVertexArray(VAO);


		// Dibujamos los puntos generados
		// por el algoritmo de punto medio

		glPointSize(2.0f);


		glDrawArrays(
			GL_POINTS,
			0,
			vertices.size() / 3
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
