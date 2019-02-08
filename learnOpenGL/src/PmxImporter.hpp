#ifndef  _PMXIMPORTER_HPP
#define  _PMXIMPORTER_HPP

#include <iostream>

#include "Mesh.h"

#include <string>
#include <vector>

#include <algorithm>

#include <cstdint>

using namespace std;

namespace yr
{
	class PmxImporter
	{
		public:
	
			PmxImporter::PmxImporter(FILE *fs, bool verbose)
			{
				this->fs = fs;
				this->verbose = verbose;
			}

			PmxImporter::PmxImporter(FILE *fs)
			{
				this->fs = fs;
			}

			PmxImporter::~PmxImporter()
			{
				free(globals);
				free(modelNameLocal);
				free(modelNameUniversal);
				free(commentsLocal);
				free(commentsUniversal);
			}

			void PmxImporter::Load()
			{
				fread(&signature, sizeof(int8_t), 4, fs);
				fread(&version, sizeof(float), 1, fs);
				fread(&globalsCount, sizeof(int8_t), 1, fs);

				cout << "Version: " << version << endl;

				globals = (int8_t*)malloc(sizeof(int8_t) * globalsCount);
				fread(globals, sizeof(int8_t), globalsCount, fs);

				// parse globals
				textEncoding = *globals;
				additionalVec4Count = *(globals + 1);
				vertexIndexSize = *(globals + 2);
				textureIndexSize = *(globals + 3);
				materialIndexSize = *(globals + 4);
				BoneIndexSize = *(globals + 5);
				MorphIndexSize = *(globals + 6);
				RigidbodyIndexSize = *(globals + 7);

				// Parse Model Name Local	
				fread(&modelNameLocalLength, sizeof(uint32_t), 1, fs);
				modelNameLocal = (int8_t*)malloc(sizeof(int8_t) * modelNameLocalLength);
				fread(modelNameLocal, sizeof(int8_t), modelNameLocalLength, fs);

				if (verbose) {
					cout << "Model Local Name: ";
					for (uint32_t i = 0; i < modelNameLocalLength; ++i) {
						cout << modelNameLocal[i];
					}
					cout << endl;
				}

				// Parse Model Name Universal	
				fread(&modelNameUniversalLength, sizeof(uint32_t), 1, fs);
				modelNameUniversal = (int8_t*)malloc(sizeof(int8_t) * modelNameUniversalLength);
				fread(modelNameUniversal, sizeof(int8_t), modelNameUniversalLength, fs);

				if (verbose) {
					cout << "Model Universal Name: ";
					for (uint32_t i = 0; i < modelNameUniversalLength; ++i) {
						cout << modelNameUniversal[i];
					}
					cout << endl;
				}

				// // Parse Model Comments Local
				fread(&commentsLocalLength, sizeof(uint32_t), 1, fs);
				commentsLocal = (int8_t*)malloc(sizeof(int8_t) * commentsLocalLength);
				fread(commentsLocal, sizeof(int8_t), commentsLocalLength, fs);

				if (verbose) {
					cout << "Model Comments Local: ";
					for (uint32_t i = 0; i < commentsLocalLength; ++i) {
						// output ascii exclude '\t\n' & control characters
						if (commentsLocal[i] <= 0x7f && commentsLocal[i] >= 0x20)
							cout << commentsLocal[i];
					}
					cout << endl;
				}

				// // Parse Model Comments Universal	
				fread(&commentsUniversalLength, sizeof(uint32_t), 1, fs);

				commentsUniversal = (int8_t*)malloc(sizeof(int8_t) * commentsUniversalLength);
				fread(commentsUniversal, sizeof(int8_t), commentsUniversalLength, fs);

				if (verbose) {
					cout << "Model Comments Universal: ";
					for (uint32_t i = 0; i < commentsUniversalLength; ++i) {
						// output ascii exclude '\t\n' & control characters
						if (commentsUniversal[i] <= 0x7f && commentsUniversal[i] >= 0x20)
							cout << commentsUniversal[i];
					}
					cout << endl;
				}

				fread(&vertexCount, sizeof(int32_t), 1, fs);

				vertexIndexSize /= 2;

				const int sizeOfBDEF1 = vertexIndexSize;
				const int sizeOfBDEF2 = vertexIndexSize * 2 + sizeof(float) * 1;
				const int sizeOfBDEF4 = vertexIndexSize * 4 + sizeof(float) * 4;
				const int sizeOfSDEF = vertexIndexSize * 2 + sizeof(float) * 1 + (sizeof(float) * 3) * 3;
				const int sizeOfQDEF = vertexIndexSize * 4 + sizeof(float) * 4;

				int32_t index1;
				int32_t index2;
				int32_t index3;
				int32_t index4;
				float boneWeight1;
				float boneWeight2;
				size_t size;

				int8_t* weightDeform = NULL;

				for (int i = 0; i < vertexCount; ++i) 
				{
					float position[3];
					float normal[3];
					float uv[2];
					float* additionalVec4 = (float*)malloc(sizeof(float) * 4 * additionalVec4Count);
					int8_t weightDeformType;
					size_t sizeOfWeightDeform;
					float edgeScale;

					size = fread(&position, sizeof(float), 3, fs);
					cout << size << endl;
					size = fread(&normal, sizeof(float), 3, fs);
					cout << size << endl;
					size = fread(&uv, sizeof(float), 2, fs);
					cout << size << endl;
					size = fread(&additionalVec4, sizeof(float), 4 * additionalVec4Count, fs);
					cout << size << endl;
					size = fread(&weightDeformType, sizeof(int8_t), 1, fs);

					switch (weightDeformType)
					{
						case 0:
							sizeOfWeightDeform = sizeOfBDEF1; break;
						case 1:
							sizeOfWeightDeform = sizeOfBDEF2; break;
						case 2:
							sizeOfWeightDeform = sizeOfBDEF4; break;
						case 3:
							sizeOfWeightDeform = sizeOfSDEF; break;
						default:
							if (version == 2.1f && weightDeformType == 4) {
								sizeOfWeightDeform = sizeOfQDEF;
							}
							else {
								// cout << "Error On Parsing Weight deform! type = " << weightDeformType << endl;
								// throw new exception();
							}
							break;
					}					

					weightDeform = (int8_t*)malloc(sizeOfWeightDeform);
					cout << size << endl;
					size = fread(weightDeform, sizeOfWeightDeform, 1, fs);
					cout << size << endl;
					size = fread(&edgeScale, sizeof(float), 1, fs);
					cout << size << endl;

					/****** Only Support Mesh for now ******/
					/*	struct Vertex vert;

					vert.Position = glm::vec3(position[0], position[1], position[2]);
					vert.Normal = glm::vec3(normal[0], normal[1], normal[2]);
					vert.TexCoords = glm::vec2(uv[0], uv[1]);
					vert.Tangent = glm::vec3(0, 0, 0);
					vert.Bitangent = glm::vec3(0, 0, 0);

					vertices.push_back(vert);*/

					// clean malloc memory

					cout << i << "(" << position[0] << ", " << position[1] << ", " << position[2] << endl;

					if (i == 28) {
						cout << "123" << endl;
					}


					if (i == 30) {
						break;
					}
				

					free(weightDeform);
					free(additionalVec4);
				}



				int32_t surfaceCount;
				fread(&surfaceCount, sizeof(int32_t), 1, fs);

				fflush(stdout);

				// free(additionalVec4);

				// system("PAUSE");
			}

		private:

			// Mesh m_mesh;

			vector<Vertex> vertices;

			bool verbose = false;
			FILE* fs;

			int8_t signature[4];	// "PMX "
			float version;			// 2.0 or 2.1
			int8_t globalsCount;	// fixed 8 for pmx 2.0
			int8_t* globals;

			// parse globals
			int8_t textEncoding;
			int8_t additionalVec4Count;
			int8_t vertexIndexSize;
			int8_t textureIndexSize;
			int8_t materialIndexSize;
			int8_t BoneIndexSize;
			int8_t MorphIndexSize;
			int8_t RigidbodyIndexSize;

			// text = int, byte[]
			uint32_t  modelNameLocalLength;
			int8_t* modelNameLocal;

			uint32_t  modelNameUniversalLength;
			int8_t* modelNameUniversal;

			uint32_t  commentsLocalLength;
			int8_t*  commentsLocal;

			uint32_t  commentsUniversalLength;
			int8_t*  commentsUniversal;

			// Vertex Data
			int32_t vertexCount;

	};

}

#endif // ! _PMXIMPORTER_HPP
