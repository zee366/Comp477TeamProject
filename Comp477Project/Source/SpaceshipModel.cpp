#include "SpaceshipModel.h"
#include "Renderer.h"
#include "ObjLoader.h"
#include "World.h"
#include "Camera.h"
#include "ThirdPersonCamera.h"
#include "FirstPersonCamera.h"
#include <glm/ext/matrix_transform.inl>
#include "EventManager.h"
#include <GLFW/glfw3.h>
#include "TextureLoader.h"
#include "PreyModel.h"

using namespace glm;
using namespace std;

SpaceshipModel::SpaceshipModel(vec3 size) : Model(), mLookAt(0.0f, 0.0f, -1.0f)
{
	std::vector<vec3> vertices;
	std::vector<vec2> uvs;
	std::vector<vec3> normals;

	ObjLoader* modelLoader = new ObjLoader();
	modelLoader->loadObj("../Assets/Models/SS1.obj", vertices, uvs, normals);
	
	//For Ufo model, uncomment below (comment above), and change in scene file: 
	// texturePath = "../Assets/Textures/s7.jpg" to texturePath = "../Assets/Textures/Ufo.png"

	//modelLoader->loadObj("../Assets/Models/Ufo.obj", vertices, uvs, normals);

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
		sizeof(vec3),		 // stride
		(void*)0             // array buffer offset
	);
	glEnableVertexAttribArray(0);

	//normals
	glBindBuffer(GL_ARRAY_BUFFER, mVBO[2]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec2), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec3),
		(void*)0     // normal is a vec3
	);
	glEnableVertexAttribArray(2);

	//uvs (coordinates)
	glBindBuffer(GL_ARRAY_BUFFER, mVBO[1]);

	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(3,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec2),
		(void*)0      // uv is a vec2
	);
	glEnableVertexAttribArray(3);

	boundingSphereRadius = 2;
	isSpaceship = true; 
}

SpaceshipModel::~SpaceshipModel()
{
	// Free the GPU from the Vertex Buffer
	glDeleteBuffers(3, &mVBO[0]);
	glDeleteVertexArrays(1, &mVAO);
}


void SpaceshipModel::Update(float dt)
{
		searching = true;

		// Logging search decision
		ci_string msg = this->GetName() + " is searching.";
		if (msg.compare(this->searchingMsg.c_str()) != 0) {
			this->searchingMsg = msg;
			World::GetInstance()->World::WriteToFile(msg.c_str());
		}

		std::vector<Model*> preylist = World::GetInstance()->getPreyList();
		std::vector<Model*> detectedPrey;
		
	int i;
	//Go over list of preys to see if any are in your sight
	for (i=0;i< preylist.size();i++)
	{
		//check if the angle between me and the prey is within my sight. 0.3 in radians == 17.1887 degrees
		if (acos(dot(normalize(mVelocity), normalize(preylist[i]->GetPosition() - mPosition))) <= 0.3)
		{
			//check if prey is being obstructed by planets, partical systems or asteroids
			if (World::GetInstance()->raycasting(mPosition, normalize(preylist[i]->GetPosition() - mPosition), preylist[i]->GetPosition()))
			{
				//added them to the list of detected preys
				detectedPrey.push_back(preylist[i]);
			}
		}
	}

	bool first = true;
	int selectedPrey;
	if (detectedPrey.size() > 0)
	{
		//Go over the list of detected preys in your sight to find the closest one
		for (i = 0; i < detectedPrey.size(); i++)

		{
			if (first)
			{
				selectedPrey = i;
				first = false;
			}

				else
				{
					if (length(detectedPrey[i]->GetPosition() - mPosition) < length(detectedPrey[selectedPrey]->GetPosition() - mPosition)) {

							dynamic_cast<PreyModel&>(*detectedPrey[selectedPrey]).SetChased(false);

							// Logging chase decision
							ci_string msgStopped = this->GetName() + " stopped chasing " + detectedPrey[selectedPrey]->GetName();
							if (msgStopped.compare(this->chasingMsg.c_str()) != 0) {
								this->chasingMsg = msgStopped;
								World::GetInstance()->World::WriteToFile(msgStopped.c_str());
							}

							selectedPrey = i;

							// Logging chase decision
							ci_string msgStarted = this->GetName() + " started chasing " + detectedPrey[selectedPrey]->GetName();
							if (msgStarted.compare(this->chasingMsg.c_str()) != 0) {
								this->chasingMsg = msgStarted;
								World::GetInstance()->World::WriteToFile(msgStarted.c_str());
							}

					}
				}
			}

			dynamic_cast<PreyModel&>(*detectedPrey[selectedPrey]).SetChased(true);

			//Set the direction of my velocity to be from me to my closest detected prey
			mVelocity = normalize(detectedPrey[selectedPrey]->GetPosition() - mPosition)*glm::length(vec3(mVelocity.x, mVelocity.y, mVelocity.z));
	
				searching = false;

				// Logging search
				ci_string msg = this->GetName() + " is not searching.";
				if (msg.compare(this->searchingMsg.c_str()) != 0) {
					this->searchingMsg = msg;
					World::GetInstance()->World::WriteToFile(msg.c_str());
				}
		}

	//if no prey is detected we continue searching
	if (searching)
	{
		bool correctionRequired = false;

		//get transformation matrix to prevent spaceship from flying into planets
		vec4 collisionCorrection = World::GetInstance()->collisionCorrection(mPosition, mVelocity)*vec4(mVelocity, 1.0f);

		// limit how often changes of dirrection can happen to avoid flikering of the spaceship
		if (directionChangeFrameCount == 0)
		{
			//changne the direction of the vector of correction is required due to an expected collision with the planates
			if (mVelocity.x != collisionCorrection.x || mVelocity.y != collisionCorrection.y || mVelocity.z != collisionCorrection.z)
			{
				mVelocity = vec3(collisionCorrection.x, collisionCorrection.y, collisionCorrection.z);
				correctionRequired = true;
				if (searchDirection == 1)
				{
					vec3 positionCollider = World::GetInstance()->getClosestModel(mPosition);
					//check weather or not we want to change the direction of the rotation depending on the position of the spaceship and the closest planet 
					if (positionCollider.x < mPosition.x || positionCollider.z > mPosition.z)
						if (directionChangeFrameCount == 0)
						{
							searchDirection = searchDirection * (-1);
							directionChangeFrameCount = 50;
						}

				}
				else
				{
					vec3 positionCollider = World::GetInstance()->getClosestModel(mPosition);
					if (positionCollider.x > mPosition.x || positionCollider.z < mPosition.z)
						if (directionChangeFrameCount == 0)
						{
							searchDirection = searchDirection * (-1);
							directionChangeFrameCount = 50;
						}
				}

			}
		}
		if (!correctionRequired)
		{
			//raguler search rotation protocal to find the next prey
			mHorizontalRotationAngleInDegrees += searchDirection*45 * dt;
			if (mHorizontalRotationAngleInDegrees > 360.0f)
				mHorizontalRotationAngleInDegrees = 0.0f;

			mVelocity.x = sin(radians(mHorizontalRotationAngleInDegrees)) * cos(radians(mVerticalRotationAngleInDegrees)) * 10;
			mVelocity.y = sin(radians(mHorizontalRotationAngleInDegrees)) * sin(radians(mVerticalRotationAngleInDegrees)) * 10;
			mVelocity.z = cos(radians(mHorizontalRotationAngleInDegrees)) * 10;
		}

	}
	
	mPosition += mVelocity * dt;

	//used to limit how often hunter will change its direction in searching to avoind flickering when a colisiion with a planat is about to occure 
	if(directionChangeFrameCount>0)
	directionChangeFrameCount--;
}

void SpaceshipModel::Draw()
{

	glActiveTexture(GL_TEXTURE0);
	GLuint textureLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "myTextureSampler");
	glBindTexture(GL_TEXTURE_2D, mTextureID);





	glBindVertexArray(mVAO);
	glBindBuffer(GL_ARRAY_BUFFER, mVAO);

	
			

		mat4 scaleMatrix =  mat4(scale(mat4(1.0f), vec3(0.01f, 0.01f, 0.01f)));
		mat4 spaceshipWorldMatrix = translate(mat4(1.0f), mPosition) * 
			rotate(mat4(1.0f), acos(dot(normalize(mVelocity), vec3(0, 0, 1))), cross(vec3(0, 0, 1), mVelocity))* //rotate based on you directional vector and the defult orientation of the spaceship <0,0,1>
			
			scaleMatrix;

		GLuint worldMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldTransform");
		glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &spaceshipWorldMatrix[0][0]);
	
	GLuint MaterialID = glGetUniformLocation(Renderer::GetShaderProgramID(), "materialCoefficients");
	glUniform4f(MaterialID, GetMaterialCoefficients().x, GetMaterialCoefficients().y, GetMaterialCoefficients().z, GetMaterialCoefficients().w);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

bool SpaceshipModel::ParseLine(const std::vector<ci_string> &token)
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
