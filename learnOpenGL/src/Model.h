#ifndef MODEL_H
#define MODEL_H

// #include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>

#include "mesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <algorithm>

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
		
		/*  Model Data */
		vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
		vector<Mesh> meshes;
		string path;
		string directory;
		FILE *fs;
		bool gammaCorrection;

		/*  Functions   */
		// constructor, expects a filepath to a 3D model.
		Model(string const &path, bool gamma = false) : path(path), gammaCorrection(gamma) { }

		// draws the model, and thus all its meshes
		/*void Draw(Shader shader)
		{
			for(unsigned int i = 0; i < meshes.size(); i++)
				meshes[i].Draw(shader);
		}*/
    
		bool Load()
		{
			// retrieve the directory path of the filepath
			directory = path.substr(0, path.find_last_of('/'));

			cout << "Open File: " << path << endl;

			errno_t err = fopen_s(&fs, path.c_str(), "r");

			cout << "File Open: " << ((err != 0) ? "FAIL" : "PASS") << endl;

			if (err != 0) {
				return false;
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
					LoadPMX();
					break;
				case MODELTYPE::PMD:
					break;
				case MODELTYPE::X:
					break;
				default:
					break;
			}

			return true;
		}
		
		void LoadPMX(bool verbose = false)
		{
			verbose = false;

			yr::PmxImporter pmxImporter(fs, verbose);

			pmxImporter.Load();
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

