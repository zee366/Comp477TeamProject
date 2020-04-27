#include "RingModel.h"
#include "Renderer.h"
#include "ObjLoader.h"
#include "World.h"
#include "Camera.h"
#include <glm/ext/matrix_transform.inl>
#include <GLFW/glfw3.h>
#include "TextureLoader.h"

using namespace glm;
using namespace std;

RingModel::RingModel(vec3 size) : Model()
{
	std::vector<vec3> vertices;
	std::vector<vec2> uvs;
	std::vector<vec3> normals;

	ObjLoader* modelLoader = new ObjLoader();
	modelLoader->loadObj("../Assets/Models/Ring.obj", vertices, uvs, normals);

	vertexCount = vertices.size();

	// Create a vertex array
	glGenVertexArrays(1, &mVAO);

	// Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
	glGenBuffers(3, &(mVBO[0]));
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	//vertices
	
	glVertexAttribPointer(0,
		3,                   // size
		GL_FLOAT,            // type
		GL_FALSE,            // normalized?
		sizeof(vec3), // stride - each vertex contain 2 vec3 (position, color)
		(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);
	//uvs (coordinates)
	glBindBuffer(GL_ARRAY_BUFFER, mVBO[1]);

	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(3,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec2),
		(void*)0      // uv is offseted a vec2
	);
	glEnableVertexAttribArray(3);
	//normals
	glBindBuffer(GL_ARRAY_BUFFER, mVBO[2]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec2), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec3),
		(void*)0     // normal is offseted a vec3
	);
}

RingModel::~RingModel()
{
	// Free the GPU from the Vertex Buffer
	glDeleteBuffers(3, &mVBO[0]);
	glDeleteVertexArrays(1, &mVAO);
}

void RingModel::Update(float dt)
{
	Model::Update(dt);
}

void RingModel::Draw()
{
	if (mTextureValid)
	{
		glActiveTexture(GL_TEXTURE0);
		GLuint textureLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "myTextureSampler");
		glBindTexture(GL_TEXTURE_2D, mTextureID);
		glUniform1i(textureLocation, 0);

	}

	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVAO);

	mat4 ringWorldMatrix = translate(mat4(1.0f), mPosition) *
		rotate(mat4(1.0f), radians(mHorizontalRotationAngleInDegrees), vec3(0.0f, 1.0f, 0.0f)) *
		scale(mat4(1.0f), vec3(0.1f, 0.1f, 0.1f));


	GLuint worldMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldTransform");
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &GetWorldMatrix()[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

bool RingModel::ParseLine(const std::vector<ci_string> &token)
{
	if (token.empty())
	{
		return true;
	}
	else
	{
		return Model::ParseLine(token);
	}
}
