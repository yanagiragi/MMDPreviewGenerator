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

		char* yrRead(void *Dst, char* Src, size_t size)
		{
			memcpy(Dst, Src, size);

			return Src + size;
		}

		void PmxImporter::Load()
		{
			fseek(fs, 0, SEEK_END);
			long fsize = ftell(fs);
			fseek(fs, 0, SEEK_SET);

			char *buffer = (char *)malloc(fsize + 1);
			fread(buffer, fsize, 1, fs);
			fclose(fs);

			buffer[fsize] = 0;

			char * current = buffer;

			current = yrRead(&signature, current, sizeof(int8_t) * 4);
			current = yrRead(&version, current, sizeof(float));
			current = yrRead(&globalsCount, current, sizeof(int8_t));

			cout << "Version: " << version << endl;

			globals = (int8_t*)malloc(sizeof(int8_t) * globalsCount);
			current = yrRead(globals, current, sizeof(int8_t) * globalsCount);
			
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
			current = yrRead(&modelNameLocalLength, current, sizeof(uint32_t));			
			modelNameLocal = (int8_t*)malloc(sizeof(int8_t) * modelNameLocalLength);
			current = yrRead(modelNameLocal, current, sizeof(int8_t) * modelNameLocalLength);
			
			if (verbose) {
				cout << "Model Local Name: ";
				for (uint32_t i = 0; i < modelNameLocalLength; ++i) {
					cout << modelNameLocal[i];
				}
				cout << endl;
			}

			// Parse Model Name Universal	
			current = yrRead(&modelNameUniversalLength, current, sizeof(uint32_t));
			modelNameUniversal = (int8_t*)malloc(sizeof(int8_t) * modelNameUniversalLength);
			current = yrRead(modelNameUniversal, current, sizeof(int8_t) * modelNameUniversalLength);
			
			if (verbose) {
				cout << "Model Universal Name: ";
				for (uint32_t i = 0; i < modelNameUniversalLength; ++i) {
					cout << modelNameUniversal[i];
				}
				cout << endl;
			}

			// // Parse Model Comments Local
			current = yrRead(&commentsLocalLength, current, sizeof(uint32_t));
			commentsLocal = (int8_t*)malloc(sizeof(int8_t) * commentsLocalLength);
			current = yrRead(commentsLocal, current, sizeof(int8_t) * commentsLocalLength);
			
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
			current = yrRead(&commentsUniversalLength, current, sizeof(uint32_t));
			commentsUniversal = (int8_t*)malloc(sizeof(int8_t) * commentsUniversalLength);
			current = yrRead(commentsUniversal, current, sizeof(int8_t) * commentsUniversalLength);

			if (verbose) {
				cout << "Model Comments Universal: ";
				for (uint32_t i = 0; i < commentsUniversalLength; ++i) {
					// output ascii exclude '\t\n' & control characters
					if (commentsUniversal[i] <= 0x7f && commentsUniversal[i] >= 0x20)
						cout << commentsUniversal[i];
				}
				cout << endl;
			}

			current = yrRead(&vertexCount, current, sizeof(int32_t));


			// hacks
			const int vertexIndexSizeForDeform = vertexIndexSize / 2;

			const int sizeOfBDEF1 = vertexIndexSizeForDeform;
			const int sizeOfBDEF2 = vertexIndexSizeForDeform * 2 + sizeof(float) * 1;
			const int sizeOfBDEF4 = vertexIndexSizeForDeform * 4 + sizeof(float) * 4;
			const int sizeOfSDEF = vertexIndexSizeForDeform * 2 + sizeof(float) * 1 + (sizeof(float) * 3) * 3;
			const int sizeOfQDEF = vertexIndexSizeForDeform * 4 + sizeof(float) * 4;

			int32_t index1;
			int32_t index2;
			int32_t index3;
			int32_t index4;
			float boneWeight1;
			float boneWeight2;
			size_t size;

			int8_t* weightDeform = NULL;

			for (int32_t i = 0; i < vertexCount; ++i)
			{
				float position[3];
				float normal[3];
				float uv[2];
				float* additionalVec4 = (float*)malloc(sizeof(float) * 4 * additionalVec4Count);
				int8_t weightDeformType;
				size_t sizeOfWeightDeform;
				float edgeScale;

				current = yrRead(&position, current, sizeof(float) * 3);
				current = yrRead(&normal, current, sizeof(float) * 3);
				current = yrRead(&uv, current, sizeof(float) * 2);
				current = yrRead(&additionalVec4, current, sizeof(float) * 4 * additionalVec4Count);
				current = yrRead(&weightDeformType, current, sizeof(int8_t));

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
							cout << "Error On Parsing Weight deform! type = " << weightDeformType << endl;
							throw new exception();
						}
						break;
				}

				weightDeform = (int8_t*)malloc(sizeOfWeightDeform);
				
				current = yrRead(weightDeform, current, sizeOfWeightDeform);
				current = yrRead(&edgeScale, current, sizeof(float));

				/****** Only Support Mesh for now ******/
				struct Vertex vert;

				vert.Position = glm::vec3(position[0], position[1], position[2]);
				vert.Normal = glm::vec3(normal[0], normal[1], normal[2]);
				vert.TexCoords = glm::vec2(uv[0], uv[1]);
				vert.Tangent = glm::vec3(0, 0, 0);
				vert.Bitangent = glm::vec3(0, 0, 0);

				vertices.push_back(vert);

				if (verbose) {
					cout << "Position [ " << i << "]: " << "(" << position[0] << ", " << position[1] << ", " << position[2] << ")" << endl;
				}
				
				// clean malloc memory
				free(weightDeform);
				free(additionalVec4);
			}



			int32_t surfaceCount;
			//fread(&surfaceCount, sizeof(int32_t), 1, fs);
			current = yrRead(&surfaceCount, current, sizeof(int32_t));

			for (int32_t i = 0; i < surfaceCount; ++i)
			{
				int32_t index;
				current = yrRead(&index, current, vertexIndexSize);
				indices.push_back(index);
			}

			// Read Materials
		}

		void DebugObj()
		{
			// write to obj for testing

			FILE *newfs = fopen("test.obj", "w");

			for (int i = 0; i < vertices.size(); ++i) {
				fprintf(newfs, "v %f %f %f\n", vertices[i].Position[0], vertices[i].Position[1], vertices[i].Position[2]);
			}


			for (int i = 0; i < vertices.size(); ++i) {
				fprintf(newfs, "vt %f %f\n", vertices[i].TexCoords[0], vertices[i].TexCoords[1]);
			}


			for (int i = 0; i < vertices.size(); ++i) {
				fprintf(newfs, "vn %f %f %f\n", vertices[i].Normal[0], vertices[i].Normal[1], vertices[i].Normal[2]);
			}

			for (int i = 0; i < indices.size(); i += 3) {
				fprintf(newfs, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
					indices[i] + 1,
					indices[i] + 1,
					indices[i] + 1,
					indices[i + 1] + 1,
					indices[i + 1] + 1,
					indices[i + 1] + 1,
					indices[i + 2] + 1,
					indices[i + 2] + 1,
					indices[i + 2] + 1);
			}

			fclose(newfs);

		}

	private:

		// Mesh m_mesh;

		vector<Vertex> vertices;
		vector<int> indices;

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
