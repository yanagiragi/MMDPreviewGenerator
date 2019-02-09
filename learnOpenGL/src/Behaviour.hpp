#ifndef BEHAVIOUR_H
#define BEHAVIOUR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

		// use int instead of unsigned to use special value -1
		vector<int> texIDs;

		unsigned int vertexShader;
		unsigned int fragmentShader;
		unsigned int shaderProgram;

		// hack = 2
		// std::string modelPath = "D:\\_Repo\\Github\\MMDPreviewGenerator\\learnOpenGL\\Resources\\TDA\ China\ Dress\ Yan\ He\ Canary\ Ver1.00\\haku.pmx";;

		wstring modelPath = L"C:\\Temp\\8\\8.pmx";
		//wstring modelPath = L"C:\\Temp\\符华玄衣素裳\\符华loli配布版.pmx";
		//wstring modelPath = L"C:\\Temp\\PPK\\ポプ子.pmx";
		//wstring modelPath = L"C:\\Temp\\haku\\haku.pmx";

		Model model = Model(this->modelPath);
		Camera mainCamera = Camera();		

		Behaviour()
		{
				
		}

		// Prepare data
		void Behaviour :: Start()
		{
			// Camera Setups
			mainCamera.eyet = 180;
			mainCamera.eyey = 10;
			mainCamera.eyez = -35;

			// Shader Setups
			Shader generator = Shader();			
			generator.CreateShader(vertexShader, GL_VERTEX_SHADER, generator.LoadRawShader("shaders/simple.vert").c_str());
			generator.CreateShader(fragmentShader, GL_FRAGMENT_SHADER, generator.LoadRawShader("shaders/simple.frag").c_str());
			generator.CreateProgram(shaderProgram, 2, vertexShader, fragmentShader);

			// create buffers/arrays			
			model.SetupMeshes(VAOs, VBOs, EBOs);
			model.SetupTextures(texIDs);

			cout << "=============== END ================" << endl;
		}

		
		void Behaviour :: Update()
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

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
					
					if(texID == -1)
						glBindTexture(GL_TEXTURE_2D, NULL);
					else
						glBindTexture(GL_TEXTURE_2D, texID);
				}

				// GL_TEXTURE0
				glUniform1i(mainTexLocation, 0);

				glBindVertexArray(VAOs[i]);
				glDrawElements(GL_TRIANGLES, model.splittedMeshs[i].indices.size(), GL_UNSIGNED_INT, 0);
			}

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
			
		}

		void Behaviour :: Destroy()
		{
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
		}
};

#endif