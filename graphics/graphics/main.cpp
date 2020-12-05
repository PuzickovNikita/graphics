#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
	glfwInit();
//проверить без этих трех команд
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//поставить GL_FALSE и порастягивать окно
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

//создаем окно
	GLFWwindow* window = glfwCreateWindow(1600, 900, "Graphics", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "failed to open window" << std::endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
//инициализируем GLEW
	glewExperimental = GL_TRUE;
	int glewInit_res = glewInit();;
	if (glewInit_res != GLEW_OK)
	{
		std::cout << "failed to initialize GLEW " << glewInit_res  << std::endl;
		//return 2;
	}
//Задаем размеры окна
	int view_w, view_h;
	glfwGetFramebufferSize(window, &view_w, &view_h);
	glViewport(0, 0, view_w, view_h);

//Основной цикл
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}