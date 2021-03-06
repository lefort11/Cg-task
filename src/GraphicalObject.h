#ifndef GRAPHICALOBJECT_H
#define GRAPHICALOBJECT_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include <freeimage.h>

#include "Vertex.h"
#include "Shader.h"
#include "Camera.h"
#include "LightingTechnique.h"

bool LoadOBJ(const char * path, std::vector<Vertex>& vertices)
{
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::vector<glm::vec3> out_vertices;
	std::vector<glm::vec2> out_uvs;
	std::vector<glm::vec3> out_normals;

	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file\n");
		getchar();
		return false;
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader

		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			//uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);

	}
	for(int i = 0; i < out_vertices.size(); ++i)
		vertices.push_back( { out_vertices[i],
							  out_normals[i],
							  {},
							  out_uvs[i],
							  {} } );

	//indices = vertexIndices;

	return true;
}

class MeshBuffer
{

	GLuint m_VertexBuffer;
	GLuint m_ElementBuffer;

	GLuint m_VertexArray; //VAO

	unsigned m_VertexNumber;
	unsigned m_ElementNumber;

public:
	MeshBuffer(Vertex const* vertices, unsigned const verticesNumber,
		 GLuint const* indices, unsigned const indicesNumber): m_ElementNumber(indicesNumber),
															   m_VertexNumber(verticesNumber)
	{
		//Setting up VAO
		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		//Setting up vertices buffer
		glGenBuffers(1, &m_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, verticesNumber * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		//Setting up indices
		glGenBuffers(1, &m_ElementBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNumber * sizeof(GLuint), indices, GL_STATIC_DRAW);


		glBindVertexArray(0);

	}

	MeshBuffer(std::vector<Vertex> const& vertices): m_ElementNumber((unsigned int)vertices.size()),
											   m_VertexNumber((unsigned int)vertices.size())
	{
		//Setting up VAO
		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		//Setting up vertices buffer
		glGenBuffers(1, &m_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		GLuint* indices = new GLuint[vertices.size()];
		for(GLuint i = 0; i < vertices.size(); ++i)
			indices[i] = i;

		//Setting up indices
		glGenBuffers(1, &m_ElementBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices.size() * sizeof(GLuint), indices, GL_STATIC_DRAW);

		glBindVertexArray(0);

	}


	~MeshBuffer()
	{
		glDeleteBuffers(1, &m_VertexBuffer);
		glDeleteBuffers(1, &m_ElementBuffer);
	}



	void Draw() const
	{

		glBindVertexArray(m_VertexArray); //VAO

		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

		//Vertex positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

		//Vertex normal
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_Normal));

		//Vertex color
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_Color));

		//Tex coords
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_TexCoords));

		//tangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_Tangent));

		//Bitangent
//		glEnableVertexAttribArray(5);
//		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, m_Bitangent));

		//indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ElementBuffer);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glDrawElements(GL_TRIANGLES, m_ElementNumber, GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
//		glDisableVertexAttribArray(5);

		glBindVertexArray(0);

	}

};

struct Material
{
	glm::vec4 Specular;
	glm::vec4 Ambient;
	glm::vec4 Diffuse;
	float Shininess;

	std::vector<LightingTechnique*> m_LightningTechniques;

public:

	Material(glm::vec4 specular = glm::vec4(0.9,0.9,0.9,1.0),
			 glm::vec4 ambient = glm::vec4(0.2,0.2,0.2,1.0),
			 glm::vec4 diffuse = glm::vec4(0.7,0.7,0.7,1.0),
			 float shininess  = 0.0):
			Specular(specular),
			Ambient(ambient),
			Diffuse(diffuse),
			Shininess(shininess)
	{}


	void LoadLightningTechniques(std::vector<LightingTechnique*> lightningTechniques)
	{
		m_LightningTechniques = lightningTechniques;
	}

	void AddLightningTechnique(LightingTechnique& lightningTechnique)
	{
		m_LightningTechniques.push_back(lightningTechnique.Clone());
	}

	void InitLightningTechniques(Shader const& shader)
	{
		for(int i = 0; i < m_LightningTechniques.size(); ++i)
		{
			m_LightningTechniques[i]->Init(shader);
		}
	}

	void DoStuff(Shader const& shader, glm::mat4 const& model) const
	{
		for(int i = 0; i < m_LightningTechniques.size(); ++i)
		{
			m_LightningTechniques[i]->DoStuff(shader, model);
		}
	}



};



class GraphicalObject
{
	MeshBuffer m_MeshBuffer;
	glm::vec3 m_CenterOffset;
	glm::vec3 m_Rotatation;
	glm::vec3 m_Scale;

	Shader* m_pShader;

	Material* m_pMaterial;



public:
	GraphicalObject(Vertex const* vertices, unsigned const verticesNumber, GLuint const* indices,
					unsigned const indicesNumber, glm::vec3 const& offset = {}, glm::vec3 const& rotate = glm::vec3(0.0f),
					glm::vec3 scale = glm::vec3(1.0f)):
			m_MeshBuffer(vertices,  verticesNumber, indices, indicesNumber),
			m_CenterOffset(offset), m_Rotatation(rotate), m_Scale(scale)
	{}

	GraphicalObject(std::vector<Vertex> vertices, glm::vec3 const& offset = {}, glm::vec3 const& rotate = glm::vec3(0.0f),
					glm::vec3 scale = glm::vec3(1.0f)):
			m_MeshBuffer(vertices), m_CenterOffset(offset), m_Rotatation(rotate), m_Scale(scale)
	{}


	void Rotate(glm::vec3 rotation)
	{
		m_Rotatation = rotation;
	}


	~GraphicalObject()
	{

	}

	void LoadShader(Shader& shader)
	{
		m_pShader = &shader;
	}

	void LoadMaterial(Material& material)
	{
		m_pMaterial = &material;
		m_pMaterial->InitLightningTechniques(*m_pShader);

	}


	void Draw(Camera const& camera) const
	{
		m_pShader->UseProgram();

		glm::mat4 mvp;
		glm::mat4 model = glm::translate(m_CenterOffset) * glm::toMat4(glm::quat(m_Rotatation)) * glm::scale(m_Scale);
		camera.GetMVP(mvp, model);


		glm::mat4 view;
		camera.GetView(view);

		glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3x3(model)));

		glUniformMatrix4fv(m_pShader->MVPID(), 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(m_pShader->ModelID(), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix3fv(m_pShader->NormalMatID(), 1, GL_FALSE, &normalMat[0][0]);

		glUniformMatrix4fv(m_pShader->ViewID(), 1, GL_FALSE, &view[0][0]);

		m_MeshBuffer.Draw();

		glUseProgram(0);

	}

	void DrawIlluminated(Camera const& camera, glm::vec4 const& lightDirection) const
	{
		m_pShader->UseProgram();

		glm::mat4 mvp;
		glm::mat4 model = glm::translate(m_CenterOffset) * glm::toMat4(glm::quat(m_Rotatation)) * glm::scale(m_Scale);
		camera.GetMVP(mvp, model);

		glm::mat4 view;
		camera.GetView(view);

		glm::mat4 lightViewMat = glm::lookAt(-glm::vec3(lightDirection), glm::vec3(0.0f, 0.0f, 0.0f),
											 {0.0f, 1.0f, 0.0f});

//		glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3x3(lightViewMat * model)));

		glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3x3(view * model)));

		glUniformMatrix4fv(m_pShader->MVPID(), 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(m_pShader->ModelID(), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix3fv(m_pShader->NormalMatID(), 1, GL_FALSE, &normalMat[0][0]);

		glUniformMatrix4fv(m_pShader->ViewID(), 1, GL_FALSE, &view[0][0]);

		glUniform4fv(m_pShader->LightDirectionID(), 1, &lightDirection[0]);

		glUniform4fv(m_pShader->MaterialSpecularID(), 1, &(m_pMaterial->Specular[0]));
		glUniform4fv(m_pShader->MaterialDiffuseID(), 1, &(m_pMaterial->Diffuse[0]));
		glUniform4fv(m_pShader->MaterialAmbientID(), 1, &(m_pMaterial->Ambient[0]));
		glUniform1f(m_pShader->MaterialShininessID(), m_pMaterial->Shininess);
		glUniformMatrix4fv(m_pShader->LightViewID(), 1, GL_FALSE, &lightViewMat[0][0]);

		m_pMaterial->DoStuff(*m_pShader, model);

		m_MeshBuffer.Draw();

		glUseProgram(0);
	}


	void DeleteShader()
	{
		m_pShader->DeleteProgram();
	}

	void GetModelMatrix(glm::mat4& model) const
	{
		model = glm::translate(m_CenterOffset) * glm::toMat4(glm::quat(m_Rotatation)) * glm::scale(m_Scale);
	}


};

#endif //GRAPHICALOBJECT_H
