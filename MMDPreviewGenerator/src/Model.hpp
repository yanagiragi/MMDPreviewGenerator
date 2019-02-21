#ifndef MODEL_HPP
#define MODEL_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STBI_WINDOWS_UTF8

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

#include "mesh.h"
#include "PmxImporter.hpp"

#include <locale>

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

		const wstring enumString[4] = { L"PMX", L"PMD", L"X", L"LENGTH" };
		
		wstring path;
		wstring directory;

		Mesh rawMesh;

		vector <Mesh> splittedMeshs;
		vector<Material> splittedMaterials;

		// flag for pmx text-encoding
		// change after LoadPmx()
		bool useWChar = false;

		inline bool exists(const wchar_t *name) {
			FILE * fs;
			errno_t err = _wfopen_s(&fs, name, L"r");
			if (!err) {
				fclose(fs);
				return true;
			}
			return false;
		}

		void tolower(wstring str)
		{
			std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		}

		/*  Functions   */
		// constructor, expects a filepath to a 3D model.		
		Model()
		{
			this->path = GlobalConfigs::wfilenameBuffer;
			
			// retrieve the directory path of the filepath
			#ifdef  _WIN32
				directory = path.substr(0, path.find_last_of('\\'));
			#else
				directory = path.substr(0, path.find_last_of('/'));
			#endif //  _WIN32
			
			Load();
		}

		~Model()
		{
			
		}

		void Load()
		{
			FILE *fs;

			errno_t err = _wfopen_s(&fs, path.c_str(), L"r");
			
			{
				// only use cout to output model name here.
				
				wstring string_to_convert;
				using convert_type = std::codecvt_utf8<wchar_t>;
				std::wstring_convert<convert_type, wchar_t> converter;
				std::string converted_str = converter.to_bytes(string_to_convert);

				wcout << "Open File (wstring): " << this->path << endl;
				
				// place holder for wstring control characters
				cout << "\t\t\t\t" << endl;
				
				cout << "Open File (utf-8): " << converted_str << endl;
				
				wcout << "Status: " << ((err != 0) ? L"FAIL" : L"PASS") << endl;
			}

			if (err != 0) {
				return;
			}

			wstring type = path.substr(path.find_last_of('.'), path.length());
			tolower(type);

			MODELTYPE modelType = MODELTYPE::LENGTH;
			if (type == L".pmx")
				modelType = MODELTYPE::PMX;
			else if (type == L".pmd")
				modelType = MODELTYPE::PMD;
			else if (type == L".x")
				modelType = MODELTYPE::X;

			wcout << L"Model Type: " << type << L"(" << enumString[static_cast<int>(modelType)] << L")" << endl << endl;

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
			yr::PmxImporter pmxImporter(fs, false);
			pmxImporter.Load(this->rawMesh);

			useWChar = pmxImporter.textEncoding == 0;

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

				wstring widetexPath;
				if (useWChar) {
					widetexPath = rawMesh.textures[i].wpath;
				}
				else {
					wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
					widetexPath = converter.from_bytes(texPath);
				}				

				if (!exists(widetexPath.c_str())) {
					wstring tmp = directory + L"\\" + widetexPath;
					if (!exists(tmp.c_str())) {
						wcout << L"Missing Texture: " + widetexPath << endl;
						textureIDs.push_back(-1);
						continue;
					}
					widetexPath = tmp;
				}

				wstring texType = widetexPath.substr(widetexPath.find_last_of('.') + 1, widetexPath.length());
				tolower(texType);

				int w, h, n;

				char buffer[GlobalConfigs::wfilenameBufferLength * 2];
				int res = stbi_convert_wchar_to_utf8(buffer, GlobalConfigs::wfilenameBufferLength * 2, widetexPath.c_str());

				unsigned char *data = stbi_load(buffer, &w, &h, &n, 0);

				// Create one OpenGL texture
				GLuint textureID;
				glGenTextures(1, &textureID);

				// "Bind" the newly created texture : all future texture functions will modify this texture
				glBindTexture(GL_TEXTURE_2D, textureID);

				GLenum internalFormat;
				GLenum format;

				bool noSupport = false;

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
					noSupport = true;
					wcout << L"Error When Parsing Texture Internal Format: internalFormat" << endl;
				}

				if (texType == L"bmp")
				{
					format = GL_BGR;
				}
				else if (texType == L"jpg")
				{
					format = (n == 4 ? GL_RGBA : GL_RGB);
				}
				else if (texType == L"jpeg")
				{
					format = (n == 4 ? GL_RGBA : GL_RGB);
				}
				else if (texType == L"PNG")
				{
					format = (n == 4 ? GL_RGBA : GL_RGB);
				}
				else if (texType == L"png")
				{
					format = (n == 4 ? GL_RGBA : GL_RGB);
				}
				else if (texType == L"tga")
				{
					format = (n == 4 ? GL_RGBA : GL_RGB);
				}
				else
				{
					noSupport = true;
					wcout << L"Error When Parsing Texture Internal Format: texType" << endl;
				}

				// Give the image to OpenGL
				if(!noSupport)
					glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				glBindTexture(GL_TEXTURE_2D, NULL);

				textureIDs.push_back(textureID);

				stbi_image_free(data);
			}
		}
};

#endif

