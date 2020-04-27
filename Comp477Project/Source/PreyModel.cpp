#include "PreyModel.h"
#include "Renderer.h"
#include "ObjLoader.h"
#include "World.h"
#include "Camera.h"
#include <glm/ext/matrix_transform.inl>
#include <GLFW/glfw3.h>
#include "TextureLoader.h"

using namespace glm;
using namespace std;

PreyModel::PreyModel(vec3 size) : Model()
{
	std::vector<vec3> vertices;
	std::vector<vec2> uvs;
	std::vector<vec3> normals;

	ObjLoader* modelLoader = new ObjLoader();
	modelLoader->loadObj("../Assets/Models/Ufo.obj", vertices, uvs, normals);

	vertexCount = vertices.size();

	int i;
	for (i = 0; i < 18; i++)
		reachedStorms.push_back(false);

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

	boundingSphereRadius = 1;
	isPrey = true; 
}

PreyModel::~PreyModel()
{
	// Free the GPU from the Vertex Buffer
	glDeleteBuffers(3, &mVBO[0]);
	glDeleteVertexArrays(1, &mVAO);
}

void PreyModel::SetChased(bool chased) {
	this->chased = chased; 
}

void PreyModel::Update(float dt)
{
	int k;
	int test = 0;
	//checked if all the storms where already reached to start over flying to the next closest storm when being chased
	for (k = 0; k < 18; k++)
	{
		if (reachedStorms[k])
			test++;
	}

	if (test == 18)
	{
		for (k = 0; k < 18; k++)
			reachedStorms[k] = false;
	}

	


	if (chased)
	{
		//escape mechanisem by flying throw the next closest storm in an orederly manner 
		std::vector<Model*> cubeList = World::GetInstance()->getCubeList(); //the cubes are used to create and position the storms

		//check if you are close engough to the storm and marked as reached to move to the next one
		for (k = 0; k < 18; k++)
		{
			if (!reachedStorms[k])
			{
				if (length(cubeList[k]->GetPosition() - mPosition) < 3)
					reachedStorms[k] = true;
			}
		}
	
		float closestCube = -1;
		int i; 
		int closestCubeIndex = -1; 

		//find the next closest storm in the list that was not reached
		for (i = 0; i < cubeList.size(); i++) {
			if (!reachedStorms[i])
			{
				//first one is selected in the first loop
				if (closestCube < 0) {
					closestCube = length(cubeList[i]->GetPosition() - mPosition);
					closestCubeIndex = i;
				}
				else if (length(cubeList[i]->GetPosition() - mPosition) < closestCube)
				{
					closestCubeIndex = i;
					closestCube = length(cubeList[i]->GetPosition() - mPosition);
				}
			}
			
				 
		}
		
		if (closestCubeIndex >= 0) {
			//Set the direction to your next closest storm
			mVelocity = normalize(cubeList[closestCubeIndex]->GetPosition() - mPosition) * 10.0f;
		}

	}
	else {

		//raguler flying in a circle protocal when not being chased anymore
		mHorizontalRotationAngleInDegrees += 45 * dt;
		if (mHorizontalRotationAngleInDegrees > 360.0f)
			mHorizontalRotationAngleInDegrees = 0.0f;

		float radius = 5;

		mVelocity.x = sin(radians(mHorizontalRotationAngleInDegrees)) * cos(radians(mVerticalRotationAngleInDegrees)) * radius;
		mVelocity.y = sin(radians(mHorizontalRotationAngleInDegrees)) * sin(radians(mVerticalRotationAngleInDegrees)) * radius;
		mVelocity.z = cos(radians(mHorizontalRotationAngleInDegrees)) * radius;
	}
	mPosition += mVelocity * dt;

}

void PreyModel::Draw()
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

	mat4 PreyWorldMatrix = translate(mat4(1.0f), mPosition) *
		rotate(mat4(1.0f), acos(dot(normalize(mVelocity), vec3(0, 0, 1))), cross(vec3(0, 0, 1), mVelocity))*//rotate based on you directional vector and the defult orientation of the spaceship <0,0,1>
		scale(mat4(1.0f), vec3(0.01f, 0.01f, 0.01f));


	GLuint worldMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldTransform");
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &PreyWorldMatrix[0][0]);

	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

bool PreyModel::ParseLine(const std::vector<ci_string> &token)
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
