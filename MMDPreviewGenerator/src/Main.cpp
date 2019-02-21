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
	GlobalConfigs::width = width;
	GlobalConfigs::height = height;
	GlobalConfigs::aspect = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

void PrintHint()
{
	cout << 
"\n================================\n\
\nMMD Preview Generator \n\
\nAuthor: yanagiragi \n\
\n================================ \n\
\nUsage: MMDPreviewGenerator.exe width height modelPath previewImageStorePath \n\
\nExamples: MMDPreviewGenerator.exe 1920 1440 \"C:\\Temp\\8\\8.pmx\" \"C:\\Temp\\8\\8_preview.png\" \n\
\n" << endl;
}

//int main(int argc, char** argv)
int wmain(int argc, wchar_t **argv, wchar_t **envp)
{
	if (argc != 5) {
		PrintHint();
		return 1;
	}
	else {

		GlobalConfigs::width = stoi(argv[1]);
		GlobalConfigs::height = stoi(argv[2]);
		GlobalConfigs::aspect = (float)GlobalConfigs::width / (float)GlobalConfigs::height;

		if (wcslen(argv[3]) < GlobalConfigs::wfilenameBufferLength) {
			wcsncpy(GlobalConfigs::wfilenameBuffer, argv[3], wcslen(argv[3]));
		}

		if (wcslen(argv[4]) < GlobalConfigs::wfilenameBufferLength) {
			wcsncpy(GlobalConfigs::wStorePathBuffer, argv[4], wcslen(argv[4]));
		}
	}
	
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
	glfwWindowHint(GLFW_VISIBLE, 0);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(GlobalConfigs::width, GlobalConfigs::height, GlobalConfigs::windowName, NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	// glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwHideWindow(window);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}	
	
	// Init before create window
	Behaviour mono = Behaviour();
	mono.Start();

	// remove comment to force resolution without occlusion
	//glfwShowWindow(window);

	// render loop
	// while (!glfwWindowShouldClose(window))
	{
		// Deal Input
		mono.Input(window);

		// Main Render Function
		mono.Update();

		mono.ScreenShot(true);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		
		glfwPollEvents();
	}

	mono.Destroy();

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}
