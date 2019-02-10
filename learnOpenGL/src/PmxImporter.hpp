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
			// fixed index size for int16_t
			int realVertexIndexSize = 2 *sizeof(int8_t);

			const int sizeOfBDEF1 = realVertexIndexSize;
			const int sizeOfBDEF2 = realVertexIndexSize * 2 + sizeof(float) * 1;
			const int sizeOfBDEF4 = realVertexIndexSize * 4 + sizeof(float) * 4;
			const int sizeOfSDEF = realVertexIndexSize * 2 + sizeof(float) * 1 + (sizeof(float) * 3) * 3;
			const int sizeOfQDEF = realVertexIndexSize * 4 + sizeof(float) * 4;

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

			// hacks
			int8_t realTextureIndexSize = textureIndexSize;

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
			
			// hacks
			int8_t realMaterialIndexSize = materialIndexSize;

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
				current = yrRead(&textureIndex, current, realTextureIndexSize);
				current = yrRead(&environmentIndex, current, realTextureIndexSize);
				current = yrRead(&environentBlendMode, current, sizeof(int8_t));

				current = yrRead(&toonReference, current, sizeof(int8_t));
				
				if (toonReference == 1) {
					current = yrRead(&toonValue, current, sizeof(int8_t));
				}
				else {
					current = yrRead(&toonValue, current, realMaterialIndexSize);
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
				
				switch (realTextureIndexSize)
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

			// no bone, morpg for now
			return;

			// Read Bone
			int32_t boneLength;
			current = yrRead(&boneLength, current, sizeof(int32_t));

			// hacks
			int8_t realBoneIndexSize = BoneIndexSize;

			for (int32_t i = 0; i < boneLength; ++i)
			{
				uint32_t boneNameLocalLength;
				int8_t* boneNameLocal;

				current = yrRead(&boneNameLocalLength, current, sizeof(uint32_t));
				boneNameLocal = (int8_t*)malloc(sizeof(int8_t) * boneNameLocalLength);
				current = yrRead(boneNameLocal, current, sizeof(int8_t) * boneNameLocalLength);

				if (verbose) {
					cout << "Bone Local [" + i << "] : ";
					for (uint32_t j = 0; j < boneNameLocalLength; ++j) {
						// output ascii exclude '\t\n' & control characters
						if (boneNameLocal[j] <= 0x7f && boneNameLocal[j] >= 0x20)
							cout << boneNameLocal[j];
					}
					cout << endl;
				}

				uint32_t boneNameUniversalLength;
				int8_t* boneNameUniversal;

				current = yrRead(&boneNameUniversalLength, current, sizeof(uint32_t));
				boneNameUniversal = (int8_t*)malloc(sizeof(int8_t) * boneNameUniversalLength);
				current = yrRead(boneNameUniversal, current, sizeof(int8_t) * boneNameUniversalLength);

				if (verbose) {
					cout << "Bone Universal [" + i << "] : ";
					for (uint32_t j = 0; j < boneNameUniversalLength; ++j) {
						// output ascii exclude '\t\n' & control characters
						if (boneNameUniversal[j] <= 0x7f && boneNameUniversal[j] >= 0x20)
							cout << boneNameUniversal[j];
					}
					cout << endl;
				}

				float position[3];
				current = yrRead(position, current, sizeof(float) * 3);

				int32_t parentBoneIndex;
				current = yrRead(&parentBoneIndex, current, realBoneIndexSize);
				parentBoneIndex = formIndex(parentBoneIndex, realBoneIndexSize);
					
				int32_t layer;
				current = yrRead(&layer, current, sizeof(int32_t));
				
				int8_t flags[2];
				current = yrRead(flags, current, sizeof(int8_t) * 2);

				int8_t indexedTailPosition	= flags[0] & 1;
				int8_t rotatable			= flags[0] & 2;
				int8_t translatable			= flags[0] & 4;
				int8_t isVisible			= flags[0] & 8;
				int8_t enabled				= flags[0] & 16;
				int8_t IK					= flags[0] & 32;

				int8_t inheritRotation		= flags[1] & 1;
				int8_t inheritTranslation	= flags[1] & 2;
				int8_t fixedAxis			= flags[1] & 4;
				int8_t localCoordinate		= flags[1] & 8;
				int8_t physicAfterDeform	= flags[1] & 16;
				int8_t externalParentDeform = flags[1] & 32;

				float tailPosition[3];
				int32_t tailBoneIndex;
				if (indexedTailPosition == 0) {
					current = yrRead(tailPosition, current, sizeof(float) * 3);
				}
				else {
					current = yrRead(&tailBoneIndex, current, realBoneIndexSize);
					tailBoneIndex = formIndex(tailBoneIndex, realBoneIndexSize);
				}

				int32_t parentIndex;
				float parentInfluence;
				if (inheritRotation != 0 || inheritTranslation != 0)
				{	
					current = yrRead(&parentIndex, current, realBoneIndexSize);
					parentIndex = formIndex(parentIndex, realBoneIndexSize);
					
					current = yrRead(&parentInfluence, current, sizeof(float));
				}

				float axisDirection[3];
				if (fixedAxis != 0)
				{
					current = yrRead(axisDirection, current, sizeof(float) * 3);
				}

				float xVector[3];
				float zVector[3];
				if (localCoordinate != 0)
				{				
					current = yrRead(xVector, current, sizeof(float) * 3);
					current = yrRead(zVector, current, sizeof(float) * 3);
				}

				if (physicAfterDeform != 0)
				{

				}

				int32_t externalParentIndex;
				if (externalParentDeform != 0)
				{
					current = yrRead(&externalParentIndex, current, realBoneIndexSize);
					externalParentIndex = formIndex(externalParentIndex, realBoneIndexSize);
				}

				int32_t IKTargetIndex;
				int32_t loopCount;
				float limitRadian;
				int32_t linkCount;

				if (IK != 0)
				{
					current = yrRead(&IKTargetIndex, current, realBoneIndexSize);
					IKTargetIndex = formIndex(IKTargetIndex, realBoneIndexSize);

					current = yrRead(&loopCount, current, sizeof(int32_t));
					current = yrRead(&limitRadian, current, sizeof(float));
					current = yrRead(&linkCount, current, sizeof(int32_t));

					for (int32_t j = 0; j < linkCount; ++j)
					{
						int32_t boneIndex;
						current = yrRead(&boneIndex, current, realBoneIndexSize);
						boneIndex = formIndex(boneIndex, realBoneIndexSize);

						int8_t hasLimits;
						current = yrRead(&hasLimits, current, sizeof(int8_t));

						float limitMin[3];
						float limitMax[3];

						if (hasLimits != 0)
						{
							current = yrRead(limitMin, current, sizeof(float) * 3);
							current = yrRead(limitMax, current, sizeof(float) * 3);
						}
					}
				}
			}

			// Read Morph
			int32_t morphLength;
			current = yrRead(&morphLength, current, sizeof(int32_t));
			
			// hacks
			int8_t realMorphIndexSize = MorphIndexSize;
			// reset realVertexIndexSize to vertexIndexSize
			// special hacks for "TDA China Princess Haku Ver 1.0\haku.pmx"
			realVertexIndexSize = vertexIndexSize;

			for (int32_t i = 0; i < morphLength; ++i)
			{
				uint32_t morphNameLocalLength;
				int8_t* morphNameLocal;

				current = yrRead(&morphNameLocalLength, current, sizeof(uint32_t));
				morphNameLocal = (int8_t*)malloc(sizeof(int8_t) * morphNameLocalLength);
				current = yrRead(morphNameLocal, current, sizeof(int8_t) * morphNameLocalLength);

				if (verbose) {
					cout << "Morph Local [" + i << "] : ";
					for (uint32_t j = 0; j < morphNameLocalLength; ++j) {
						// output ascii exclude '\t\n' & control characters
						if (morphNameLocal[j] <= 0x7f && morphNameLocal[j] >= 0x20)
							cout << morphNameLocal[j];
					}
					cout << endl;
				}

				uint32_t morphNameUniversalLength;
				int8_t* morphNameUniversal;

				current = yrRead(&morphNameUniversalLength, current, sizeof(uint32_t));
				morphNameUniversal = (int8_t*)malloc(sizeof(int8_t) * morphNameUniversalLength);
				current = yrRead(morphNameUniversal, current, sizeof(int8_t) * morphNameUniversalLength);

				if (verbose) {
					cout << "Morph Universal [" + i << "] : ";
					for (uint32_t j = 0; j < morphNameUniversalLength; ++j) {
						// output ascii exclude '\t\n' & control characters
						if (morphNameUniversal[j] <= 0x7f && morphNameUniversal[j] >= 0x20)
							cout << morphNameUniversal[j];
					}
					cout << endl;
				}

				int8_t panelType;
				int8_t morphType;
				int32_t offsetSize;

				current = yrRead(&panelType, current, sizeof(int8_t));
				current = yrRead(&morphType, current, sizeof(int8_t));
				current = yrRead(&offsetSize, current, sizeof(int32_t));

				for (int32_t j = 0; j < offsetSize; ++j)
				{
					switch (morphType)
					{
						case 0: // Group
							int32_t groupIndex;
							float influence;
							current = yrRead(&groupIndex, current, realMorphIndexSize);
							groupIndex = formIndex(groupIndex, realMorphIndexSize);
							current = yrRead(&influence, current, sizeof(float));
							break;
						case 1: // Vertex
							int32_t vertexIndex;
							float translation[3];
							current = yrRead(&vertexIndex, current, realVertexIndexSize);
							vertexIndex = formIndex(vertexIndex, realVertexIndexSize);
							current = yrRead(translation, current, sizeof(float) * 3);
							break;
						case 2: // Bone
							int32_t morphBoneIndex;
							float morphBonetranslation[3];
							float morphBonerotation[4];
							current = yrRead(&morphBoneIndex, current, realBoneIndexSize);
							morphBoneIndex = formIndex(morphBoneIndex, realBoneIndexSize);
							current = yrRead(morphBonetranslation, current, sizeof(float) * 3);
							current = yrRead(morphBonerotation, current, sizeof(float) * 4);
							break;
						case 3: // UV
						case 4: // UV ext1
						case 5: // UV ext2
						case 6: // UV ext3
						case 7: // UV ext4
							int32_t UVVertexIndex;
							float floats[4];
							current = yrRead(&UVVertexIndex, current, realVertexIndexSize);
							UVVertexIndex = formIndex(UVVertexIndex, realVertexIndexSize);
							current = yrRead(floats, current, sizeof(float) * 4);
							break;
						case 8: // Matreial
							int32_t materialIndex;
							int8_t IsNotMultiplicative; //  0 for multiplicative, non-zero for additive

							float diffuseColour[4];
							float specularColour[3];
							float specularStrength;
							float ambientColour[3];
							float edgeColour[4];
							float edgeScale;
							float textureTint[4];
							float environmentTint[4];
							float toonTint[4];
							current = yrRead(&materialIndex, current, realMaterialIndexSize);
							materialIndex = formIndex(materialIndex, realMaterialIndexSize);
							current = yrRead(&IsNotMultiplicative, current, sizeof(int8_t));
							current = yrRead(&diffuseColour, current, sizeof(float) * 4);
							current = yrRead(&specularColour, current, sizeof(float) * 3);
							current = yrRead(&specularStrength, current, sizeof(float));
							current = yrRead(&ambientColour, current, sizeof(float) * 3);
							current = yrRead(&edgeColour, current, sizeof(float) * 4);
							current = yrRead(&edgeScale, current, sizeof(float));
							current = yrRead(&toonTint, current, sizeof(float) * 4);
							current = yrRead(&environmentTint, current, sizeof(float) * 4);
							current = yrRead(&toonTint, current, sizeof(float) * 4);

							// Do Something with the material, no use for now
							//if (materialIndex != -1)
							//{
							//	// is multi
							//	if (j == 0) {
							//		if(IsNotMultiplicative)
							//			mesh.materials[materialIndex].diffuse.a += diffuseColour[4];
							//		else
							//			mesh.materials[materialIndex].diffuse.a *= diffuseColour[4];
							//	}
							//		
							//	else {
							//		if (IsNotMultiplicative)
							//			mesh.materials[materialIndex].diffuse.a += diffuseColour[4];
							//		else
							//			mesh.materials[materialIndex].diffuse.a *= diffuseColour[4];
							//	}
							//}
							//else {
							//	for (int k = 0; k < mesh.materials.size(); ++k)
							//	{
							//		if (IsNotMultiplicative)
							//			mesh.materials[k].diffuse.a += diffuseColour[4];
							//		else
							//			mesh.materials[k].diffuse.a *= diffuseColour[4];
							//	}
							//}

							break;
						case 9: // version 2.1, Flip
							throw new exception("Version 2.0 does not support morph type = flip!");
							break;
						case 10: // version 2.1, Impulse
							throw new exception("Version 2.0 does not support morph type = Impulse!");
							break;
						default: // Error
							throw new exception("Paring Morph Type Error!");
							break;
					}
				}

				cout << "123" << endl;
			}


			// Display Frame, Rigid Body, Soft Bidy, etc.
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
