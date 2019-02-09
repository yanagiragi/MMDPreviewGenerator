#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <io.h>
#include <fcntl.h>
#include <Windows.h>

#include "Configs.hpp"

#include <iostream>
#include "Shader.h"
#include "Behaviour.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

int main(int argc, char** argv)
{
	// _setmode(_fileno(stdout), _O_U16TEXT);

	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(GlobalConfigs::width, GlobalConfigs::height, GlobalConfigs::windowName, NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwHideWindow(window);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	
	// Init before create window
	Behaviour mono = Behaviour();
	mono.Start();

	glfwShowWindow(window);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// Deal Input
		mono.Input(window);

		// Main Render Function
		mono.Update();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	mono.Destroy();

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}
