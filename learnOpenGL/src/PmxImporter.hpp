#ifndef _PMXIMPORTER_HPP
#define _PMXIMPORTER_HPP

#include <locale>
#include <codecvt>

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "Mesh.h"

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

		int32_t formIndex(int32_t index, int8_t indexSize)
		{
			switch (indexSize)
			{
				case 1:
					index = static_cast<int8_t>(index);
					break;
				case 2:
					index = static_cast<int16_t>(index);
					break;
				case 4:
					index = static_cast<int32_t>(index);
					break;
				default:
					break;
			}

			return index;
		}

		void PmxImporter::Load(Mesh &mesh)
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

			cout << "Vertex Count = " << vertexCount << endl;

			// hacks
			const int vertexIndexSizeForDeform = 2;

			const int sizeOfBDEF1 = vertexIndexSizeForDeform;
			const int sizeOfBDEF2 = vertexIndexSizeForDeform * 2 + sizeof(float) * 1;
			const int sizeOfBDEF4 = vertexIndexSizeForDeform * 4 + sizeof(float) * 4;
			const int sizeOfSDEF = vertexIndexSizeForDeform * 2 + sizeof(float) * 1 + (sizeof(float) * 3) * 3;
			const int sizeOfQDEF = vertexIndexSizeForDeform * 4 + sizeof(float) * 4;

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

				mesh.vertices.push_back(vert);

				// clean malloc memory
				free(weightDeform);
				free(additionalVec4);
			}

			// read face data
			current = yrRead(&surfaceCount, current, sizeof(int32_t));

			cout << "Surface Count = " << surfaceCount << endl;

			for (int32_t i = 0; i < surfaceCount; ++i)
			{
				int32_t index;
				current = yrRead(&index, current, vertexIndexSize);
				index = formIndex(index, vertexIndexSize);
				mesh.indices.push_back(static_cast<unsigned int>(index));
			}

			// Read Texture
			current = yrRead(&textureCount, current, sizeof(int32_t));

			cout << "Texture Count = " << textureCount << endl;

			for (int32_t i = 0; i < textureCount; ++i)
			{
				int8_t* texturePath;
				uint32_t texturePathLength;
				current = yrRead(&texturePathLength, current, sizeof(uint32_t));
				texturePath = (int8_t*)malloc(sizeof(int8_t) * texturePathLength + sizeof(int8_t));
				current = yrRead(texturePath, current, sizeof(int8_t) * texturePathLength);

				wchar_t *wchartexturePath = (wchar_t*)malloc(sizeof(int8_t) * texturePathLength);
				memcpy(wchartexturePath, texturePath, sizeof(int8_t) * texturePathLength);

				wchartexturePath[sizeof(int8_t) * texturePathLength / sizeof(char16_t)] = L'\0';
				
				wstring wtexturePath = wstring(wchartexturePath);
				
				string texturePathStr;				
				for (uint32_t j = 0; j < texturePathLength; ++j) {
					// output ascii exclude '\t\n' & control characters
					if (texturePath[j] <= 0x7f && texturePath[j] >= 0x20)
						texturePathStr += texturePath[j];
				}

				struct Texture tex;
				tex.path = texturePathStr;
				tex.wpath = wtexturePath;
				tex.id = i;
				mesh.textures.push_back(tex);

				if (verbose) {
					cout << "Found Texture: " << texturePathStr << endl;
				}

				// free(wchartexturePath);
				free(texturePath);
			}

			// Read Materials
			current = yrRead(&materialCount, current, sizeof(int32_t));
			
			cout << "Material Count = " << materialCount << endl;

			for (int32_t i = 0; i < materialCount; ++i)
			{
				uint32_t materialNameLocalLength;
				int8_t* materialNameLocal;

				current = yrRead(&materialNameLocalLength, current, sizeof(uint32_t));
				materialNameLocal = (int8_t*)malloc(sizeof(int8_t) * materialNameLocalLength);
				current = yrRead(materialNameLocal, current, sizeof(int8_t) * materialNameLocalLength);

				if (verbose) {
					cout << "Material [" + i << "] : ";
					for (uint32_t j = 0; j < materialNameLocalLength; ++j) {
						// output ascii exclude '\t\n' & control characters
						if (materialNameLocal[j] <= 0x7f && materialNameLocal[j] >= 0x20)
							cout << materialNameLocal[j];
					}
					cout << endl;
				}

				uint32_t materialNameUniversalLength;
				int8_t* materialNameUniversal;

				current = yrRead(&materialNameUniversalLength, current, sizeof(uint32_t));
				materialNameUniversal = (int8_t*)malloc(sizeof(int8_t) * materialNameUniversalLength);
				current = yrRead(materialNameUniversal, current, sizeof(int8_t) * materialNameUniversalLength);

				if (verbose) {
					cout << "Material [" + i << "] : ";
					for (uint32_t j = 0; j < materialNameUniversalLength; ++j) {
						// output ascii exclude '\t\n' & control characters
						if (materialNameUniversal[j] <= 0x7f && materialNameUniversal[j] >= 0x20)
							cout << materialNameUniversal[j];
					}
					cout << endl;
				}

				float diffuseColour[4];
				float specularColour[3];
				float specularStrength;
				float ambientColour[3];
				int8_t drawingFlags;
				float edgeColour[4];
				float edgeScale;
				int32_t textureIndex;
				int32_t environmentIndex;
				int8_t environentBlendMode;
				int8_t toonReference;

				// if toonReference is 1, toon value will be a byte reference toon01.bmp to toon10.bmp
				// else, toon value will be a index
				int32_t toonValue;

				int32_t metaDataLength;
				int8_t* metaData;

				int32_t surfaceCount;

				current = yrRead(&diffuseColour, current, sizeof(float) * 4);
				current = yrRead(&specularColour, current, sizeof(float) * 3);
				current = yrRead(&specularStrength, current, sizeof(float));
				current = yrRead(&ambientColour, current, sizeof(float) * 3);
				current = yrRead(&drawingFlags, current, sizeof(int8_t));
				current = yrRead(&edgeColour, current, sizeof(float) * 4);
				current = yrRead(&edgeScale, current, sizeof(float));
				current = yrRead(&textureIndex, current,textureIndexSize);
				current = yrRead(&environmentIndex, current, textureIndexSize);
				current = yrRead(&environentBlendMode, current, sizeof(int8_t));

				current = yrRead(&toonReference, current, sizeof(int8_t));
				
				if (toonReference == 1) {
					current = yrRead(&toonValue, current, sizeof(int8_t));
				}
				else {
					current = yrRead(&toonValue, current, materialIndexSize);
				}

				current = yrRead(&metaDataLength, current, sizeof(int32_t));
				metaData = (int8_t*)malloc(sizeof(int8_t) * metaDataLength);
				current = yrRead(metaData, current, sizeof(int8_t) * metaDataLength);

				current = yrRead(&surfaceCount, current, sizeof(int32_t));

				struct Material mat;

				mat.diffuse = glm::vec4(diffuseColour[0], diffuseColour[1], diffuseColour[2], diffuseColour[3]);
				mat.specular = glm::vec3(specularColour[0], specularColour[1], specularColour[2]);
				mat.gloss = specularStrength;
				mat.ambient = glm::vec3(ambientColour[0], ambientColour[1], ambientColour[2]);
				mat.edgeColor = glm::vec3(edgeColour[0], edgeColour[1], edgeColour[2]);
				mat.edgeScale = edgeScale;
				
				switch (textureIndexSize)
				{
					case 1:
						textureIndex = static_cast<int8_t>(textureIndex);
						environmentIndex = static_cast<int8_t>(environmentIndex);
						break;
					case 2:
						textureIndex = static_cast<int16_t>(textureIndex); 
						environmentIndex = static_cast<int16_t>(environmentIndex);
						break;
					case 4:
						textureIndex = static_cast<int32_t>(textureIndex); 
						environmentIndex = static_cast<int32_t>(environmentIndex);
						break;
					default:
						break;
				}

				mat.textureIndex = static_cast<int>(textureIndex);
				mat.envMapIndex = static_cast<int>(environmentIndex);
				mat.surfaceCount = static_cast<unsigned int>(surfaceCount);
				
				mesh.materials.push_back(mat);
			}

			// No Support For Bone, Morph, Display Frame, Rigid Body, Soft Bidy, etc.
			DebugObj(mesh);
		}

		void DebugObj(Mesh mesh)
		{
			// write to obj for testing
			FILE *newfs;
			errno_t err = fopen_s(&newfs, "test.obj", "w");

			for (int i = 0; i < mesh.vertices.size(); ++i) {
				fprintf(newfs, "v %f %f %f\n", mesh.vertices[i].Position[0], mesh.vertices[i].Position[1], mesh.vertices[i].Position[2]);
			}


			for (int i = 0; i < mesh.vertices.size(); ++i) {
				fprintf(newfs, "vt %f %f\n", mesh.vertices[i].TexCoords[0], mesh.vertices[i].TexCoords[1]);
			}


			for (int i = 0; i < mesh.vertices.size(); ++i) {
				fprintf(newfs, "vn %f %f %f\n", mesh.vertices[i].Normal[0], mesh.vertices[i].Normal[1], mesh.vertices[i].Normal[2]);
			}

			for (int i = 0; i < mesh.indices.size(); i += 3) {
				fprintf(newfs, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
					mesh.indices[i] + 1,
					mesh.indices[i] + 1,
					mesh.indices[i] + 1,
					mesh.indices[i + 1] + 1,
					mesh.indices[i + 1] + 1,
					mesh.indices[i + 1] + 1,
					mesh.indices[i + 2] + 1,
					mesh.indices[i + 2] + 1,
					mesh.indices[i + 2] + 1);
			}

			fclose(newfs);

		}

		// parse globals
		int8_t textEncoding;
		int8_t additionalVec4Count;
		int8_t vertexIndexSize;
		int8_t textureIndexSize;
		int8_t materialIndexSize;
		int8_t BoneIndexSize;
		int8_t MorphIndexSize;
		int8_t RigidbodyIndexSize;

	private:

		bool verbose = false;
		FILE* fs;

		int8_t signature[4];	// "PMX "
		float version;			// 2.0 or 2.1
		int8_t globalsCount;	// fixed 8 for pmx 2.0
		int8_t* globals;

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

		// Surface Data
		int32_t surfaceCount;

		// Texture Data
		int32_t textureCount;

		// Texture Data
		int32_t materialCount;
	};

}

#endif // ! _PMXIMPORTER_HPP
