#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
};

struct Texture {
    unsigned int id;
    string path;
	wstring wpath;
};

struct Material {
	glm::vec4 diffuse;
	glm::vec3 specular;
	glm::vec3 ambient;
	glm::vec3 edgeColor;
	float gloss;
	float edgeScale;

	// toon value

	// avoid using unsigned int
	// since texId may equal to -1 for "No Texture"
	int textureIndex;
	int envMapIndex;
	
	unsigned int surfaceCount;
};

class Mesh {
public:
    /*  Mesh Data  */
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
	vector<Material> materials;

	Mesh()
	{

	}

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
    }

	~Mesh()
	{

	}
};
#endif
