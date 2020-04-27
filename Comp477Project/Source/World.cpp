//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "World.h"
#include "Renderer.h"
#include "ParsingHelper.h"

#include "StaticCamera.h"
#include "FirstPersonCamera.h"
#include "ThirdPersonCamera.h"
#include "MainCamera.h"

#include "CubeModel.h"
#include "SphereModel.h"
#include "RingModel.h"
#include "SpaceshipModel.h"
#include "PreyModel.h"

#include "Animation.h"
#include "Billboard.h"
#include <GLFW/glfw3.h>
#include "EventManager.h"
#include "TextureLoader.h"

#include "ParticleDescriptor.h"
#include "ParticleEmitter.h"
#include "ParticleSystem.h"

#include "BSpline.h"
#include "BSplineCamera.h"
#include "AsteroidModel.h"
#include "UI_elements.hpp"

#include <glm/gtx/transform.hpp>

#include <chrono>

#include <string>

using namespace std;
using namespace glm;

World* World::instance;

MainCamera* mp = new MainCamera(vec3(3.0f, 10.0f, 50.0f));
ThirdPersonCamera* tp = new ThirdPersonCamera(vec3(3.0f, 5.0f, 20.0f));
FirstPersonCamera* fpShip = new FirstPersonCamera(vec3(0.0f));

World::World() :
	mFirstPersonCameraTarget(0), mThirdPersonCameraTarget(0), mNumberOfHunters(0), mNumberOfPrey(0)
{
    instance = this;

	mOut.open("output.txt");

	// Setup Camera
	mCamera.push_back(mp);
	mCamera.push_back(tp);
	mCamera.push_back(fpShip);
	mCurrentCamera = 0;
    
#if defined(PLATFORM_OSX)
	int billboardTextureIDFire = TextureLoader::LoadTexture("/Textures/Fire.png");
	int billboardTextureIDSmoke = TextureLoader::LoadTexture("/Textures/Smoke.png");
	int billboardTextureIDStorm = TextureLoader::LoadTexture("/Textures/Storm.png");
#else
    int billboardTextureIDFire = TextureLoader::LoadTexture("../Assets/Textures/Fire.png");
	int billboardTextureIDSmoke = TextureLoader::LoadTexture("../Assets/Textures/Smoke.png");
	int billboardTextureIDStorm = TextureLoader::LoadTexture("../Assets/Textures/Storm.png");

 
#endif
    assert(billboardTextureIDFire != 0);
	assert(billboardTextureIDSmoke != 0);
	assert(billboardTextureIDStorm != 0);

    mpBillboardListFire = new BillboardList(2048, billboardTextureIDFire);
	mpBillboardListSmoke = new BillboardList(2048, billboardTextureIDSmoke);
	mpBillboardListStorm = new BillboardList(2048, billboardTextureIDStorm);
}

World::~World()
{
	mOut.close();

	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
		delete *it;
	mModel.clear();

	for (vector<UI_elements*>::iterator it = mUI.begin(); it < mUI.end(); ++it)
		delete *it;
	mUI.clear();

	for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
		delete *it;
	mAnimation.clear();

	for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
		delete *it;
	mAnimationKey.clear();

	for (vector<Camera*>::iterator it = mCamera.begin(); it < mCamera.end(); ++it)
		delete *it;
	mCamera.clear();
    
    for (vector<ParticleSystem*>::iterator it = mParticleSystemList.begin(); it < mParticleSystemList.end(); ++it)
		delete *it;
	mParticleSystemList.clear();
    
    for (vector<ParticleDescriptor*>::iterator it = mParticleDescriptorList.begin(); it < mParticleDescriptorList.end(); ++it)
		delete *it;
	mParticleDescriptorList.clear();

	for (vector<BSpline*>::iterator it = mSpline.begin(); it < mSpline.end(); ++it)
		delete *it;
	mSpline.clear();

	delete mpBillboardListFire;
	delete mpBillboardListSmoke;
	delete mpBillboardListStorm;
}

void World::WriteToFile(const char* str) {
	// Timestamp with milliseconds from https://gist.github.com/bschlinker/844a88c09dcf7a61f6a8df1e52af7730

	const auto now = std::chrono::system_clock::now();
	const auto nowAsTimeT = std::chrono::system_clock::to_time_t(now);
	const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
		now.time_since_epoch()) % 1000;
	std::stringstream nowSs;
	nowSs
		<< std::put_time(std::localtime(&nowAsTimeT), "%F %T")
		<< '.' << std::setfill('0') << std::setw(3) << nowMs.count();
	
	mOut << nowSs.str() << " " << str << endl;
}

std::vector<Model*> World::getPreyList()
{
	std::vector<Model*> prey;

	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		if ((*it)->isPrey)
			prey.push_back(*it);
	}
	return prey;
}

std::vector<Model*> World::getCubeList()
{
	std::vector<Model*> cube;
	bool first = true;

	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		//dynamic cast to check if the model is a cube
		if (CubeModel* cubeCheck = dynamic_cast<CubeModel*>(*it))
		{
			if (first)
				first = false;
			else
			{
				cube.push_back(*it);
			}
		}
	}
	return cube;
}

World* World::GetInstance()
{
    return instance;
}

int lastMouseButtonState = GLFW_RELEASE;
void World::Update(float dt)
{
	float dt2 = dt * speed;
	// Read mouse button. Toggle first person if RIGHT click is detected.
	if (glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		mp->toggleMouse(true);
		tp->toggleMouse(true);
	}
	else if (glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		mp->toggleMouse(false);
		tp->toggleMouse(false);
	}
	// User Inputs
	// 0 1 2 to change the Camera
	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_1 ) == GLFW_PRESS)
	{
		// main camera
		mCurrentCamera = 0;
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_2 ) == GLFW_PRESS)
	{
		if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_2) == GLFW_RELEASE) {
			if (mCamera.size() > 1)
			{
				// third person camera cycles through hunters and prey
				mCurrentCamera = 1;
				mThirdPersonCameraTarget = (mThirdPersonCameraTarget % (mNumberOfHunters + mNumberOfPrey)) + 1;
			}
		}
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_3 ) == GLFW_PRESS)
	{
		if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_3) == GLFW_RELEASE) {
			if (mCamera.size() > 2)
			{
				// first person camera cycles through hunters
				mCurrentCamera = 2;
				mFirstPersonCameraTarget = (mFirstPersonCameraTarget % mNumberOfHunters) + 1;
			}
		}
	}

	// Spacebar to change the shader
	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_0 ) == GLFW_PRESS)
	{
		Renderer::SetShader(SHADER_SOLID_COLOR);
	}
	else if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_9 ) == GLFW_PRESS)
	{
		Renderer::SetShader(SHADER_BLUE);
	}

    // Update animation and keys
    for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
    {
        (*it)->Update(dt2);
    }
    
    for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
    {
        (*it)->Update(dt2);
    }

	// Update current Camera
	mCamera[mCurrentCamera]->Update(dt);

	//Raycasting
	if (lastMouseButtonState == GLFW_RELEASE && glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS
		&& buttonState == 2) {
		for (vector<Model*>::iterator it = mModel.begin() + 2; it < mModel.end(); ++it)
		{
			if ((*it)->IntersectsRay(mp->GetPosition(), vec3(-inverse(GetCurrentCamera()->GetViewMatrix())[2])) == true) {
				mp->toggleMouse(true);
				mp->setPosition((*it)->GetPosition() + vec3(0.0f, 0.0f, 10.0f));
				mp->setLookAt((*it)->GetPosition());
				mCurrentCamera = 0;
				mCamera[mCurrentCamera]->Update(dt);
			}
		}
	}
	else if (lastMouseButtonState == GLFW_RELEASE && glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS
		&& glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		if (mCurrentCamera == 3) {
			mModel.push_back(new AsteroidModel(GetCurrentCamera()->GetPosition() + vec3(0, 0.15f, -0.1f),
				vec3(-inverse(GetCurrentCamera()->GetViewMatrix())[2]), vec3(0.1f, 0.1f, 0.1f)));
		}
		else {
			mModel.push_back(new AsteroidModel(GetCurrentCamera()->GetPosition(),
				vec3(-inverse(GetCurrentCamera()->GetViewMatrix())[2]), vec3(0.1f, 0.1f, 0.1f)));
		}
	}
	lastMouseButtonState = glfwGetMouseButton(EventManager::GetWindow(), GLFW_MOUSE_BUTTON_LEFT);

	// Update ship camera
	vec3 offset = vec3(mModel[mFirstPersonCameraTarget]->GetVelocity()) * 0.26f ;
	fpShip->setPosition(mModel[mFirstPersonCameraTarget]->GetPosition() + offset);
	fpShip->setLookAt(normalize(mModel[mFirstPersonCameraTarget]->GetVelocity() * dt));

	//Check collisions
	for (vector<Model*>::iterator it = mModel.begin() +1; it < mModel.end(); ++it)
	{
		//Intersphere collisions
		for (vector<Model*>::iterator it2 = it; it2 < mModel.end(); ++it2)
		{
			//Models can't collide with themselves, and both models cant be projectiles
			if (it != it2)
			{
				Model* m1 = *it;
				Model* m2 = *it2;

				float distance = glm::distance(m1->GetPosition(), m2->GetPosition());
				float r1 = m1->GetRadius();
				float r2 = m2->GetRadius();
				float totalRadii = r1 + r2;


				// Collisions between Models

				if (distance <= totalRadii) //Collision
				{
					if ((m1->isPrey && m2->isSpaceship) || (m2->isPrey && m1->isSpaceship))
					{
						if (m1->isPrey) {
							ci_string msg = m2->GetName() + " KILLED " + m1->GetName();
							World::WriteToFile(msg.c_str());
							mModel.erase(std::remove(mModel.begin(), mModel.end(), m1), mModel.end());
						}
						else {
							ci_string msg = m1->GetName() + " KILLED " + m2->GetName();
							World::WriteToFile(msg.c_str());
							mModel.erase(std::remove(mModel.begin(), mModel.end(), m2), mModel.end());
						}
						mNumberOfPrey--;
					}
					else if (m1->GetName().compare("\"Cube\"") != 0 && m2->GetName().compare("\"Cube\"") != 0)
					{
						glm::vec3 collisionNormal = glm::normalize(m1->GetPosition() - m2->GetPosition());
						glm::vec3 collisionPoint = m2->GetPosition() + r2 * collisionNormal;
						glm::vec3 normalVelocity1 = glm::dot(m1->GetVelocity(), collisionNormal) * collisionNormal;
						glm::vec3 normalVelocity2 = glm::dot(m2->GetVelocity(), collisionNormal) * collisionNormal;

						glm::vec3 tangentMomentum1 = m1->GetVelocity() - normalVelocity1;
						glm::vec3 tangentMomentum2 = m2->GetVelocity() - normalVelocity2;

						float mass1 = m1->GetMass();
						float mass2 = m2->GetMass();
						glm::vec3 newNormalVelocity1 = ((mass1 - mass2) / (mass1 + mass2)) * normalVelocity1 + ((2 * mass2) / (mass1 + mass2) * normalVelocity2);
						glm::vec3 newNormalVelocity2 = ((2 * mass1) / (mass1 + mass2)) * normalVelocity1 + ((mass2 - mass1) / (mass1 + mass2) * normalVelocity2);

						m1->SetVelocity(newNormalVelocity1 + tangentMomentum1);
						m2->SetVelocity(newNormalVelocity2 + tangentMomentum2);
					}
				}
			}
		}
	}

	(*mModel.begin())->SetPosition(GetCurrentCamera()->GetPosition());
	if (mCurrentCamera == 1)
	{
		vec3 shipPosition = mModel[mThirdPersonCameraTarget]->GetPosition();
		vec3 cameraLook = normalize(mCamera[mCurrentCamera]->GetLookAt());

		mCamera[mCurrentCamera]->setPosition(shipPosition - cameraLook);
		(*mModel.begin())->SetPosition(GetCurrentCamera()->GetAngledPosition());
	}

	// Update models
	for (vector<Model*>::iterator it = mModel.begin() + 1; it < mModel.end(); ++it)
	{
		(*it)->Update(dt2);
	}

    // Update billboards
    for (vector<ParticleSystem*>::iterator it = mParticleSystemList.begin(); it != mParticleSystemList.end(); ++it)
	{
		(*it)->Update(dt);
	}

	mpBillboardListStorm->Update(dt);
    mpBillboardListFire->Update(dt);
	mpBillboardListSmoke->Update(dt);
}

bool World::raycasting(glm::vec3 source, glm::vec3 direction, glm::vec3 prey)
{
	
	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		//dynamic cast to check if the model is a sphere
		if (SphereModel* sphere = dynamic_cast<SphereModel*>(*it))
			//check if there if intersect ray will colide with the sphere
			if (sphere->IntersectsRay(source, direction))
				//check if the prey in behined the blocking obstacle
				if (length(prey - source) > length(sphere->GetPosition() - source))
					return false;

		//dynamic cast to check if the model is a asteroid
		if (AsteroidModel* asteroid = dynamic_cast<AsteroidModel*>(*it))
			//check if there if intersect ray will colide with the asteroid
			if (asteroid->IntersectsRay(source, direction))
				//check if the prey in behined the blocking obstacle
				if (length(prey - source) > length(asteroid->GetPosition() - source))
					return false;
	}

	for (vector<ParticleSystem*>::iterator it = mParticleSystemList.begin(); it != mParticleSystemList.end(); ++it)
	{
		if ((*it)->IntersectsRay(source, direction))
			if (length(prey - source) > length((*it)->getPosition() - source))
				return false;
	}

	return true;
}

glm::mat4 World::collisionCorrection(glm::vec3 source, glm::vec3 direction)
{
	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		//dynamic cast to check if the model is a sphere
		if (SphereModel* sphere = dynamic_cast<SphereModel*>(*it))
			//check if there if intersect ray will colide with the sphere
			if (sphere->IntersectsRay(source, direction))
				//check if the spaceship is close to the sphere
				if (sphere->GetBoundingRadius()!=0 && ((sphere->GetBoundingRadius()*3.0f) > length(sphere->GetPosition() - source)) )
					return rotate(mat4(1.0f), acos(dot(normalize(sphere->GetPosition() - source), normalize(direction))), cross( normalize(sphere->GetPosition() - source), normalize(direction)));

		//dynamic cast to check if the model is a asteroid
		if (AsteroidModel* asteroid = dynamic_cast<AsteroidModel*>(*it))
			//check if there if intersect ray will colide with the asteroid
			if (asteroid->IntersectsRay(source, direction))
				//check if the spaceship is close to the sphere
				if ((asteroid->GetBoundingRadius()*(1.1f)) > length(asteroid->GetPosition() - source))
					return rotate(mat4(1.0f), acos(dot(normalize(direction), normalize(asteroid->GetPosition() - source))), cross(normalize(direction), normalize(asteroid->GetPosition() - source)));
	}

	return mat4(1.0f);
}

glm::vec3 World::getClosestModel(glm::vec3 source)
{
	bool first = true;
	vec3 position;

	for (vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		//dynamic cast to check if the model is a sphere
		if (SphereModel* sphere = dynamic_cast<SphereModel*>(*it))
		{ 
			if (first)
			{
				first = false;
				position = sphere->GetPosition();
			}
			else
			{
				if(length(position-source)>length(sphere->GetPosition()-source))
					position = sphere->GetPosition();
			}
		}

		//dynamic cast to check if the model is a asteroid
		if (AsteroidModel* asteroid = dynamic_cast<AsteroidModel*>(*it))
		{
			if (first)
			{
				first = false;
				position = asteroid->GetPosition();
			}
			else
			{
				if (length(position - source) > length(asteroid->GetPosition() - source))
					position = asteroid->GetPosition();
			}
		}
	}

	return position;
}

void World::Draw(float dt)
{
	Renderer::BeginFrame();
	
	// Set shader to use
	glUseProgram(Renderer::GetShaderProgramID());

	// This looks for the MVP Uniform variable in the Vertex Program
	GLuint VPMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");

	// Send the view projection constants to the shader
	mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
	glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

	GLuint ViewMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewTransform");
	mat4 View = mCamera[mCurrentCamera]->GetViewMatrix();
	glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, &View[0][0]);

	GLuint ProjectionMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ProjectionTransform");
	mat4 Projection = mCamera[mCurrentCamera]->GetProjectionMatrix();
	glUniformMatrix4fv(ProjectionMatrixLocation, 1, GL_FALSE, &Projection[0][0]);

	// Get a handle for Light Attributes uniform
	GLuint LightPositionID = glGetUniformLocation(Renderer::GetShaderProgramID(), "WorldLightPosition");
	GLuint LightColorID = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightColor");
	GLuint LightAttenuationID = glGetUniformLocation(Renderer::GetShaderProgramID(), "lightAttenuation");

	// Light Coefficients
	const vec3 lightColor(1.0f, 1.0f, 1.0f);
	const float lightKc = 0.05f;
	const float lightKl = 0.02f;
	const float lightKq = 0.002f;
	const vec4 lightPosition(0.0f, 0.0f, 0.0f, 1.0f); // If w = 1.0f, we have a point light, w = 0.0f for directional light

	glUniform4f(LightPositionID, lightPosition.x, lightPosition.y, lightPosition.z, lightPosition.w);
	glUniform3f(LightColorID, lightColor.r, lightColor.g, lightColor.b);
	glUniform3f(LightAttenuationID, lightKc, lightKl, lightKq);

	unsigned int prevCamera = mCurrentCamera;
	bool firstPass = true;
	for (int i = 0; i < 2; ++i) {
		if (!firstPass) {
			mCurrentCamera = 2;
			(*mModel.begin())->SetPosition(mCamera[mCurrentCamera]->GetPosition());
			View = mCamera[mCurrentCamera]->GetViewMatrix();
			glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, &View[0][0]);
			VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
			glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);
			mpBillboardListStorm->Update(dt);
			mpBillboardListFire->Update(dt);
			mpBillboardListSmoke->Update(dt);
		}

		glDisable(GL_DEPTH_TEST);
		(*mModel.begin())->Draw();
		glEnable(GL_DEPTH_TEST);
		glUseProgram(Renderer::GetShaderProgramID());

		// Draw models
		bool firstModel = true;
		for (vector<Model*>::iterator it = mModel.begin() + 1; it < mModel.end(); ++it)
		{
			(*it)->Draw();
		}

		// Draw Path Lines

		// Set Shader for path lines
		Renderer::SetShader(SHADER_PATH_LINES);
		glUseProgram(Renderer::GetShaderProgramID());

		// Send the view projection constants to the shader
		VPMatrixLocation = glGetUniformLocation(Renderer::GetShaderProgramID(), "ViewProjectionTransform");
		glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

		for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
		{
			mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
			glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

			(*it)->Draw();
		}

		for (vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
		{
			mat4 VP = mCamera[mCurrentCamera]->GetViewProjectionMatrix();
			glUniformMatrix4fv(VPMatrixLocation, 1, GL_FALSE, &VP[0][0]);

			(*it)->Draw();
		}

		// Draw Billboards
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		mpBillboardListStorm->Draw();
		mpBillboardListFire->Draw();
		mpBillboardListSmoke->Draw();

		glDisable(GL_BLEND);

		if (firstPass) {
			glClear(GL_DEPTH_BUFFER_BIT);
			Renderer::EnableSecondaryViewport(scrn_width, scrn_height);
			firstPass = false;
		}

		// Restore previous shader
		Renderer::SetShader(SHADER_TEXTURED);
		glUseProgram(Renderer::GetShaderProgramID());
	}
	Renderer::EnablePrimaryViewport(scrn_width, scrn_height);
	mCurrentCamera = prevCamera;

	Renderer::EndFrame();
}

void World::LoadScene(const char * scene_path)
{
	// Using case-insensitive strings and streams for easier parsing
	ci_ifstream input;
	input.open(scene_path, ios::in);

	// Invalid file
	if(input.fail() )
	{	 
		fprintf(stderr, "Error loading file: %s\n", scene_path);
		getchar();
		exit(-1);
	}

	ci_string item;
	while( std::getline( input, item, '[' ) )   
	{
        ci_istringstream iss( item );

		ci_string result;
		if( std::getline( iss, result, ']') )
		{
			if( result == "cube" )
			{
				// Box attributes
				CubeModel* cube = new CubeModel();
				cube->Load(iss);
				mModel.push_back(cube);
			}
            else if( result == "sphere" )
            {
                SphereModel* sphere = new SphereModel();
                sphere->Load(iss);
                mModel.push_back(sphere);
            }
			else if (result == "ring")
			{
				RingModel* ring = new RingModel();
				ring->Load(iss);
				mModel.push_back(ring);
			}
			else if (result == "asteroid")
			{
				AsteroidModel* asteroid = new AsteroidModel();
				asteroid->Load(iss);
				mModel.push_back(asteroid);
			}
			else if (result == "spaceship")
			{
				SpaceshipModel* spaceship = new SpaceshipModel();
				spaceship->isSpaceship = true;
				spaceship->Load(iss);
				spaceship->SetScaling(vec3(0.1f, 0.1f, 0.1f));
				mModel.push_back(spaceship);
				mNumberOfHunters++;
			}
			else if (result == "prey")
			{
				PreyModel* prey = new PreyModel();
				prey->Load(iss);
				mModel.push_back(prey);
				mNumberOfPrey++;
			}

			else if ( result == "animationkey" )
			{
				AnimationKey* key = new AnimationKey();
				key->Load(iss);
				mAnimationKey.push_back(key);
			}
			else if (result == "animation")
			{
				Animation* anim = new Animation();
				anim->Load(iss);
				mAnimation.push_back(anim);
			}
			else if (result == "spline")
			{
				BSpline* spline = new BSpline();
				spline->Load(iss);
				spline->CreateVertexBuffer();

				// This is hardcoded: replace last camera with spline camera
				mSpline.push_back(spline);
				mCamera.pop_back();
				mCamera.push_back(new BSplineCamera(spline, 10.0f));
			}
            else if (result == "particledescriptor")
            {
                ParticleDescriptor* psd = new ParticleDescriptor();
                psd->Load(iss);
                AddParticleDescriptor(psd);
            }
			else if ( result.empty() == false && result[0] == '#')
			{
				// this is a comment line
			}
			else
			{
				fprintf(stderr, "Error loading scene file... !");
				getchar();
				exit(-1);
			}
	    }
	}
	input.close();

	// Set Animation vertex buffers
	for (vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
	{
		// Draw model
		(*it)->CreateVertexBuffer();
	}

	mFirstPersonCameraTarget = mNumberOfHunters;
	mThirdPersonCameraTarget = mNumberOfHunters + mNumberOfPrey;
}

Animation* World::FindAnimation(ci_string animName)
{
    for(std::vector<Animation*>::iterator it = mAnimation.begin(); it < mAnimation.end(); ++it)
    {
        if((*it)->GetName() == animName)
        {
            return *it;
        }
    }
    return nullptr;
}

AnimationKey* World::FindAnimationKey(ci_string keyName)
{
    for(std::vector<AnimationKey*>::iterator it = mAnimationKey.begin(); it < mAnimationKey.end(); ++it)
    {
        if((*it)->GetName() == keyName)
        {
            return *it;
        }
    }
    return nullptr;
}

Model* World::FindParentModel(ci_string name)
{
	for (std::vector<Model*>::iterator it = mModel.begin(); it < mModel.end(); ++it)
	{
		if ((*it)->GetName() == name)
		{
			return *it;
		}
	}
	return nullptr;
 }

const Camera* World::GetCurrentCamera() const
{
     return mCamera[mCurrentCamera];
}

void World::AddBillboard(Billboard* b, ci_string name)
{
	if(name.compare("\"Fountain\"") == 0)
    mpBillboardListFire->AddBillboard(b);
	else if (name.compare("\"Smoke\"") == 0)
	mpBillboardListSmoke->AddBillboard(b);
	else if (name.compare("\"Storm\"") == 0)
	mpBillboardListStorm->AddBillboard(b);
}

void World::RemoveBillboard(Billboard* b, ci_string name)
{
	if (name.compare("\"Fountain\"") == 0)
		mpBillboardListFire->RemoveBillboard(b);
	else if (name.compare("\"Smoke\"") == 0)
		mpBillboardListSmoke->RemoveBillboard(b);
	else if (name.compare("\"Storm\"") == 0)
		mpBillboardListStorm->RemoveBillboard(b);
}

void World::AddParticleSystem(ParticleSystem* particleSystem)
{
    mParticleSystemList.push_back(particleSystem);
}

void World::RemoveParticleSystem(ParticleSystem* particleSystem)
{
    vector<ParticleSystem*>::iterator it = std::find(mParticleSystemList.begin(), mParticleSystemList.end(), particleSystem);
    mParticleSystemList.erase(it);
}

void World::AddParticleDescriptor(ParticleDescriptor* particleDescriptor)
{
    mParticleDescriptorList.push_back(particleDescriptor);
}

ParticleDescriptor* World::FindParticleDescriptor(ci_string name)
{
    for(std::vector<ParticleDescriptor*>::iterator it = mParticleDescriptorList.begin(); it < mParticleDescriptorList.end(); ++it)
    {
        if((*it)->GetName() == name)
        {
            return *it;
        }
    }
    return nullptr;
}
