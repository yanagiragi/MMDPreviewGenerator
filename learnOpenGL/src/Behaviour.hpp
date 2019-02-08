﻿#ifndef BEHAVIOUR_H
#define BEHAVIOUR_H

//#include <windows.h>
//#include <gl/GL.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Model.h"
#include "Camera.hpp"

class Behaviour 
{
	public:
		unsigned int VAO, VBO, EBO;
		unsigned int vertexShader;
		unsigned int fragmentShader;
		unsigned int shaderProgram;

		Model ppk = Model("D:\\_Repository\\Github Projects\\MMDPreviewGenerator\\learnOpenGL\\Resources\\haku\\haku.pmx");

		Camera mainCamera = Camera();

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
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, ppk.mesh.vertices.size() * sizeof(Vertex), &ppk.mesh.vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, ppk.mesh.indices.size() * sizeof(unsigned int), &ppk.mesh.indices[0], GL_STATIC_DRAW);

			// set the vertex attribute pointers
			// vertex Positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			// vertex normals
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			// vertex texture coords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
			// vertex tangent
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
			// vertex bitangent
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

			glBindVertexArray(0);

		}

		void Behaviour :: Update()
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			glEnable(GL_DEPTH_TEST);

			glUseProgram(shaderProgram);
			
			int MVPLocation = glGetUniformLocation(shaderProgram, "MVP");

			glm::mat4 M(1.0f);
			//M = glm::translate(M, glm::vec3(0, -10, -25));
			glm::mat4 V = mainCamera.getV();
			glm::mat4 P = mainCamera.getP();
			glm::mat4 MVP = P * V * M;

			glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &MVP[0][0]);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, ppk.mesh.indices.size(), GL_UNSIGNED_INT, 0);

		/*	for (int i = 0; i < ppk.mesh.indices.size(); i += 3) {
				int index0 = ppk.mesh.indices[i];
				int index1 = ppk.mesh.indices[i + 1];
				int index2 = ppk.mesh.indices[i + 2];

				glBegin(GL_TRIANGLES);
				glVertex3f(ppk.mesh.vertices[index0].Position.x, ppk.mesh.vertices[index0].Position.y, ppk.mesh.vertices[index0].Position.z);
				glVertex3f(ppk.mesh.vertices[index1].Position.x, ppk.mesh.vertices[index1].Position.y, ppk.mesh.vertices[index1].Position.z);
				glVertex3f(ppk.mesh.vertices[index2].Position.x, ppk.mesh.vertices[index2].Position.y, ppk.mesh.vertices[index2].Position.z);
				glEnd();
			}*/

		}

		// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
		void Behaviour::Input(GLFWwindow *window)
		{
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				glfwSetWindowShouldClose(window, true);

			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				mainCamera.eyez += mainCamera.step;
			else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				mainCamera.eyez -= mainCamera.step;
			else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				mainCamera.eyex -= mainCamera.step;
			else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				mainCamera.eyex += mainCamera.step;
			else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
				mainCamera.eyey += mainCamera.step;
			else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
				mainCamera.eyey -= mainCamera.step;

			else if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
				mainCamera.eyet += mainCamera.step;
			else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
				mainCamera.eyet -= mainCamera.step;
			else if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
				mainCamera.eyep -= mainCamera.step;
			else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
				mainCamera.eyep += mainCamera.step;
			
		}

		void Behaviour :: Destroy()
		{
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
		}
};

#endif