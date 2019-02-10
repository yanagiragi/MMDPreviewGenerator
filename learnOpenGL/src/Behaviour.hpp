#ifndef BEHAVIOUR_H
#define BEHAVIOUR_H

// #include <glad/glad.h>
//#include <GLFW/glfw3.h>
//
//#include <Magick++.h>

#include "Shader.h"
#include "Model.hpp"
#include "Camera.hpp"

class Behaviour
{
public:
	unsigned int VAO, VBO, EBO;

	vector<unsigned int> VAOs;
	vector<unsigned int> VBOs;
	vector<unsigned int> EBOs;
	GLuint renderedTexture;
	GLuint frameBuffer = 0;

	// use int instead of unsigned to use special value -1
	vector<int> texIDs;

	unsigned int vertexShader;
	unsigned int fragmentShader;
	unsigned int shaderProgram;

	// filepath store in GlobalConfigs::wfilenameBuffer (read from argv[1])
	Model model = Model();

	Camera mainCamera = Camera();

	Behaviour()
	{

	}

	void Behaviour::ScreenShot()
	{
		const int format_nchannels = 3;
		GLubyte *pixels;
		pixels = (GLubyte*)malloc(sizeof(GLubyte) * GlobalConfigs::width * GlobalConfigs::height * format_nchannels);

		memset(pixels, 0, GlobalConfigs::width * GlobalConfigs::height * format_nchannels * sizeof(GLubyte));

		//glReadBuffer(frameBuffer);

		// glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, GlobalConfigs::width, GlobalConfigs::height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		// glGetTexImage(renderedTexture, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		char storeFilePath[GlobalConfigs::wfilenameBufferLength * 2];
		int result = stbiw_convert_wchar_to_utf8(storeFilePath, GlobalConfigs::wfilenameBufferLength * 2, GlobalConfigs::wStorePathBuffer);

		// stbi_flip_vertically_on_write(1);			
		result = stbi_write_png(storeFilePath, GlobalConfigs::width, GlobalConfigs::height, format_nchannels, pixels, 0);

		/*FILE *f = fopen((string(storeFilePath) + string(".ppm")).c_str(), "w");
		int width = GlobalConfigs::width;
		int height = GlobalConfigs::height;
		fprintf(f, "P3\n%d\n%d\n%d\n", width, height, 255);
		pixels = (GLubyte*)realloc(pixels, format_nchannels * sizeof(GLubyte) * width * height);
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		int cur = 0;
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				cur = format_nchannels * ((height - i - 1) * width + j);
				fprintf(f, "%3d %3d %3d ", (pixels)[cur], (pixels)[cur + 1], (pixels)[cur + 2]);
			}
			fprintf(f, "\n");
		}
		fclose(f);*/

		wcout << storeFilePath << " Stored." << endl;

		free(pixels);

		exit(0);
	}

	// Prepare data
	void Behaviour::Start()
	{
		// Camera Setups
		mainCamera.eyet = 180;
		mainCamera.eyey = 9.899;
		mainCamera.eyez = -32.400;

		// Shader Setups
		Shader generator = Shader();
		generator.CreateShader(vertexShader, GL_VERTEX_SHADER, generator.LoadRawShader("shaders/simple.vert").c_str());
		generator.CreateShader(fragmentShader, GL_FRAGMENT_SHADER, generator.LoadRawShader("shaders/simple.frag").c_str());
		generator.CreateProgram(shaderProgram, 2, vertexShader, fragmentShader);

		// create buffers/arrays			
		model.SetupMeshes(VAOs, VBOs, EBOs);
		model.SetupTextures(texIDs);

		// Render Texture Setup
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		glGenTextures(1, &renderedTexture);
		glBindTexture(GL_TEXTURE_2D, renderedTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GlobalConfigs::width, GlobalConfigs::height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "FrameBuffer Not Ready" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		glBindTexture(GL_TEXTURE_2D, NULL);

		cout << "=============== END ================" << endl;
	}

	void Behaviour::Update()
	{
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		// glLoadIdentity();

		// glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(shaderProgram);

		int MVPLocation = glGetUniformLocation(shaderProgram, "MVP");
		int diffuseColorLocation = glGetUniformLocation(shaderProgram, "diffuseColor");
		int specularColorLocation = glGetUniformLocation(shaderProgram, "specularColor");
		int glossLocation = glGetUniformLocation(shaderProgram, "gloss");
		int ambientColorLocation = glGetUniformLocation(shaderProgram, "ambientColor");
		int mainTexLocation = glGetUniformLocation(shaderProgram, "mainTex");

		glm::mat4 M(1.0f);
		//M = glm::translate(M, glm::vec3(0, -10, -25));
		glm::mat4 V = mainCamera.getV();
		glm::mat4 P = mainCamera.getP();
		glm::mat4 MVP = P * V * M;

		glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &MVP[0][0]);

		for (int i = 0; i < VAOs.size(); ++i)
		{
			Material mat = model.splittedMaterials[i];

			glUniform4f(diffuseColorLocation, mat.diffuse.r, mat.diffuse.g, mat.diffuse.b, mat.diffuse.a);
			glUniform3f(specularColorLocation, mat.specular.r, mat.specular.g, mat.specular.b);
			glUniform1f(glossLocation, mat.gloss);
			glUniform3f(ambientColorLocation, mat.ambient.r, mat.ambient.g, mat.ambient.b);

			if (mat.textureIndex == -1)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, NULL);
			}
			else {
				unsigned int texID = texIDs[mat.textureIndex];

				glActiveTexture(GL_TEXTURE0);

				if (texID == -1)
					glBindTexture(GL_TEXTURE_2D, NULL);
				else
					glBindTexture(GL_TEXTURE_2D, texID);
			}

			// GL_TEXTURE0
			glUniform1i(mainTexLocation, 0);

			glBindVertexArray(VAOs[i]);
			glDrawElements(GL_TRIANGLES, model.splittedMeshs[i].indices.size(), GL_UNSIGNED_INT, 0);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		glBindTexture(GL_TEXTURE_2D, NULL);

		//GLubyte *pixels;
		//pixels = (GLubyte*)malloc(sizeof(GLubyte) * GlobalConfigs::width * GlobalConfigs::height * 3);

		//memset(pixels, 0, GlobalConfigs::width * GlobalConfigs::height * 3 * sizeof(GLubyte));

		//// glGetTexImage(renderedTexture, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		//glRasterPos2i(-1, -1);
		//glDrawPixels(GlobalConfigs::width, GlobalConfigs::height, GL_RGB, GL_FLOAT, pixels);

		//cout << "tick\n" << endl;

	}

	// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
	void Behaviour::Input(GLFWwindow *window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			mainCamera.eyez -= mainCamera.step;
		else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			mainCamera.eyez += mainCamera.step;
		else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			mainCamera.eyex -= mainCamera.step;
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			mainCamera.eyex += mainCamera.step;
		else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			mainCamera.eyey += mainCamera.step;
		else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			mainCamera.eyey -= mainCamera.step;

		else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
			mainCamera.eyep += mainCamera.step;
		else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
			mainCamera.eyep -= mainCamera.step;
		else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
			mainCamera.eyet -= mainCamera.step;
		else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
			mainCamera.eyet += mainCamera.step;

		else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
			ScreenShot();
		
		// cout << "eye position = (" << mainCamera.eyex << ", " << mainCamera.eyey << ", " << mainCamera.eyez << ") ( p = " << mainCamera.eyep << ", t = " << mainCamera.eyet << ")" << endl;
	}

	void Behaviour::Destroy()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
};

#endif