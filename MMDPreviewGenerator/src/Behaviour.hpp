#ifndef BEHAVIOUR_H
#define BEHAVIOUR_H

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
	GLuint renderedTexture, depthTexture;
	GLuint frameBuffer = 0;

	// use int instead of unsigned to use special value -1
	vector<int> texIDs;

	unsigned int vertexShader;
	unsigned int fragmentShader;
	unsigned int shaderProgram;

	GLuint testTexture;

	// filepath store in GlobalConfigs::wfilenameBuffer (read from argv[1])
	Model model = Model();

	Camera mainCamera = Camera();

	Behaviour()
	{

	}

	void Behaviour::ScreenShot()
	{
		const int format_nchannels = 4;
		
		glViewport(0, 0, GlobalConfigs::width, GlobalConfigs::height);

		GLubyte *pixels;
		pixels = (GLubyte*)malloc(sizeof(GLubyte) * GlobalConfigs::width * GlobalConfigs::height * format_nchannels);
		memset(pixels, 0, GlobalConfigs::width * GlobalConfigs::height * format_nchannels * sizeof(GLubyte));

		if (GlobalConfigs::useRenderTexture) {
			glBindBuffer(GL_READ_BUFFER, frameBuffer);
			glReadPixels(0, 0, GlobalConfigs::width, GlobalConfigs::height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		}
		else {		
			glReadPixels(0, 0, GlobalConfigs::width, GlobalConfigs::height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		}	
		
		char storeFilePath[GlobalConfigs::wfilenameBufferLength * 2];
		int result = stbiw_convert_wchar_to_utf8(storeFilePath, GlobalConfigs::wfilenameBufferLength * 2, GlobalConfigs::wStorePathBuffer);
		
		stbi_flip_vertically_on_write(1);
		result = stbi_write_png(storeFilePath, GlobalConfigs::width, GlobalConfigs::height, format_nchannels, pixels, 0);

		wcout << GlobalConfigs::wStorePathBuffer << L" Stored." << endl;

		free(pixels);
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

		if (GlobalConfigs::useRenderTexture)
		{
			// Render Texture Setup
			glGenFramebuffers(1, &frameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

			glGenTextures(1, &renderedTexture);
			glBindTexture(GL_TEXTURE_2D, renderedTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GlobalConfigs::width, GlobalConfigs::height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);

			// bind depth texutre or else depth test will not work
			glGenTextures(1, &depthTexture);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GlobalConfigs::width, GlobalConfigs::height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

			auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
				wcout << L"Framebuffer not complete: " << fboStatus << std::endl;
		}		

		wcout << L"=============== START END ================" << endl;
	}

	void Behaviour::Update()
	{
		// IMPORTANT: Clear Before Bind Buffer
		// Check: https://stackoverflow.com/a/41367665

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		if(GlobalConfigs::useRenderTexture)
			glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
				
		glUseProgram(shaderProgram);

		int MVPLocation = glGetUniformLocation(shaderProgram, "MVP");
		int diffuseColorLocation = glGetUniformLocation(shaderProgram, "diffuseColor");
		int specularColorLocation = glGetUniformLocation(shaderProgram, "specularColor");
		int glossLocation = glGetUniformLocation(shaderProgram, "gloss");
		int ambientColorLocation = glGetUniformLocation(shaderProgram, "ambientColor");
		int mainTexLocation = glGetUniformLocation(shaderProgram, "mainTex");

		glm::mat4 M(1.0f);
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

		// debug display
		if(GlobalConfigs::useRenderTexture) {
			glBindFramebuffer(GL_FRAMEBUFFER, NULL);
			glClear(GL_COLOR_BUFFER_BIT);

			float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
				// positions   // texCoords
				-1.0f,  1.0f,  0.0f, 1.0f,
				-1.0f, -1.0f,  0.0f, 0.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,

				-1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,
				 1.0f,  1.0f,  1.0f, 1.0f
			};

			unsigned int quadVAO, quadVBO;
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

			unsigned int debugVertShader;
			unsigned int debugFragShader;
			unsigned int debugshaderProgram;

			Shader generator = Shader();
			generator.CreateShader(debugVertShader, GL_VERTEX_SHADER, generator.LoadRawShader("shaders/debug.vert").c_str());
			generator.CreateShader(debugFragShader, GL_FRAGMENT_SHADER, generator.LoadRawShader("shaders/debug.frag").c_str());
			generator.CreateProgram(debugshaderProgram, 2, debugVertShader, debugFragShader);

			glUseProgram(debugshaderProgram);
			glBindVertexArray(quadVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, renderedTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthTexture);
			glUniform1i(glGetUniformLocation(debugshaderProgram, "screenTexture"), 0);
			glUniform1i(glGetUniformLocation(debugshaderProgram, "depthTexture"), 1);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
		glBindTexture(GL_TEXTURE_2D, NULL);
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
		glDeleteFramebuffers(1, &frameBuffer);
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
};

#endif