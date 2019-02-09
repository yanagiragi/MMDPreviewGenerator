#ifndef MODEL_H
#define MODEL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <iostream>

#include "mesh.h"
#include "PmxImporter.hpp"

using namespace std;

class Model 
{
	public:
		enum class MODELTYPE
		{
			PMX,
			PMD,
			X,
			LENGTH
		};

		const string enumString[4] = { "PMX", "PMD", "X", "LENGTH" };
		
		string path;
		string directory;

		Mesh rawMesh;

		vector <Mesh> splittedMeshs;
		vector<Material> splittedMaterials;

		inline bool exists(const std::string& name) {
			struct stat buffer;
			return (stat(name.c_str(), &buffer) == 0);
		}

		inline bool exists(const char *name) {
			struct stat buffer;
			return (stat(name, &buffer) == 0);
		}

		/*  Functions   */
		// constructor, expects a filepath to a 3D model.
		Model(string const &path)
		{ 
			FILE *fs;

			// retrieve the directory path of the filepath
			#ifdef  _WIN32
				directory = path.substr(0, path.find_last_of('\\'));
			#else 
				directory = path.substr(0, path.find_last_of('/'));
			#endif //  _WIN32


			errno_t err = fopen_s(&fs, path.c_str(), "r");
			cout << "File Open: " << ((err != 0) ? "FAIL" : "PASS") << endl;
			if (err != 0) {
				return;
			}

			string type = path.substr(path.find_last_of('.'), path.length());
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);

			MODELTYPE modelType = MODELTYPE::LENGTH;
			if (type == ".pmx")
				modelType = MODELTYPE::PMX;
			else if (type == ".pmd")
				modelType = MODELTYPE::PMD;
			else if (type == ".x")
				modelType = MODELTYPE::X;

			cout << "Model Type: " << type << "(" << enumString[static_cast<int>(modelType)] << ")" << endl;

			switch (modelType)
			{
				case MODELTYPE::PMX:
					LoadPMX(fs);
					break;
				case MODELTYPE::PMD:
					break;
				case MODELTYPE::X:
					break;
				default:
					break;
			}
		}

		void LoadPMX(FILE *fs, bool verbose = false)
		{
			yr::PmxImporter pmxImporter(fs, verbose);
			pmxImporter.Load(this->rawMesh);

			PostProcessPmx();
		}

		void PostProcessPmx()
		{
			// split mesh by material
			int current = 0;
			for (int i = 0; i < rawMesh.materials.size(); ++i) 
			{
				Mesh m;
				m.vertices = rawMesh.vertices;
				m.textures = rawMesh.textures;
				m.materials.push_back(rawMesh.materials[i]);
				for (int j = 0; j < rawMesh.materials[i].surfaceCount; ++j)
				{					
					m.indices.push_back(rawMesh.indices[current + j]);
				}

				splittedMeshs.push_back(m);
				current += rawMesh.materials[i].surfaceCount;
			}

			splittedMaterials = rawMesh.materials;
		}

		void SetupMeshes(vector<unsigned int> &VAOs, vector<unsigned int> &VBOs, vector<unsigned int> &EBOs)
		{
			for (int i = 0; i < splittedMeshs.size(); ++i) {

				unsigned int VAO, VBO, EBO;

				// create buffers/arrays
				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);
				glGenBuffers(1, &EBO);

				glBindVertexArray(VAO);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, splittedMeshs[i].vertices.size() * sizeof(Vertex), &splittedMeshs[i].vertices[0], GL_STATIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, splittedMeshs[i].indices.size() * sizeof(unsigned int), &splittedMeshs[i].indices[0], GL_STATIC_DRAW);

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

				VAOs.push_back(VAO);
				VBOs.push_back(VBO);
				EBOs.push_back(EBO);
			}
		}

		void SetupTextures(vector<int> &textureIDs)
		{
			for (int i = 0; i < rawMesh.textures.size(); ++i) {
				
				string texPath = rawMesh.textures[i].path;
				string texType = rawMesh.textures[i].type;

				if (!exists(texPath)) {
					texPath = directory + "\\" + texPath;
				}

				if (!exists(texPath))
				{
					cout << "Missing Texture: " + rawMesh.textures[i].path << endl;
					textureIDs.push_back(-1);
					continue;
				}

				int w, h, n;

				unsigned char *data = stbi_load(texPath.c_str(), &w, &h, &n, 0);

				// Create one OpenGL texture
				GLuint textureID;
				glGenTextures(1, &textureID);

				// "Bind" the newly created texture : all future texture functions will modify this texture
				glBindTexture(GL_TEXTURE_2D, textureID);

				GLenum internalFormat;
				GLenum format;

				if (n == 3)
				{
					internalFormat = GL_RGB;
				}
				else if (n == 4)
				{
					internalFormat = GL_RGBA;
				}
				else
				{
					cout << "Error When Parsing Texture Internal Format";
				}

				if (texType == "bmp")
				{
					format = GL_BGR;
				}
				else if (texType == "png")
				{
					format = (n == 4 ? GL_RGBA : GL_RGB);
				}
				else if (texType == "tga")
				{
					format = (n == 4 ? GL_RGBA : GL_RGB);
				}
				else
				{
					cout << "Error When Parsing Texture Internal Format";
				}

				// Give the image to OpenGL
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				glBindTexture(GL_TEXTURE_2D, NULL);

				textureIDs.push_back(textureID);
			}
		}

	private:
		

		/*unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
		{
			string filename = string(path);
			filename = directory + '/' + filename;

			unsigned int textureID;
			glGenTextures(1, &textureID);

			int width, height, nrComponents;
			unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
			if (data)
			{
				GLenum format;
				if (nrComponents == 1)
					format = GL_RED;
				else if (nrComponents == 3)
					format = GL_RGB;
				else if (nrComponents == 4)
					format = GL_RGBA;

				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				stbi_image_free(data);
			}
			else
			{
				std::cout << "Texture failed to load at path: " << path << std::endl;
				stbi_image_free(data);
			}

			return textureID;
		}*/
};

#endif

