//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "FirstPersonCamera.h"
#include "EventManager.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>
#include <algorithm>



using namespace glm;
//bool freeWill = false;

FirstPersonCamera::FirstPersonCamera(glm::vec3 position) :  Camera(), mPosition(position), mLookAt(0.0f, 0.0f, -1.0f), mHorizontalAngle(90.0f), mVerticalAngle(0.0f), mSpeed(5.0f), mAngularSpeed(2.5f)
{
}

FirstPersonCamera::~FirstPersonCamera()
{
}

void FirstPersonCamera::Update(float dt)
{	
	EventManager::EnableMouseCursor();

	// Default W S D A commands. 
	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_W) == GLFW_PRESS)
		mPosition += dt * mSpeed * vec3(0.0f, 0.0f, -1.0f);

	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_S) == GLFW_PRESS)
		mPosition -= dt * mSpeed * vec3(0.0f, 0.0f, -1.0f);

	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_D) == GLFW_PRESS)
		mPosition += dt * mSpeed* vec3(1.0f, 0.0f, 0.0f);

	if (glfwGetKey(EventManager::GetWindow(), GLFW_KEY_A) == GLFW_PRESS)
		mPosition -= dt * mSpeed* vec3(1.0f, 0.0f, 0.0f);
}

glm::mat4 FirstPersonCamera::GetViewMatrix() const
{
	return glm::lookAt(	mPosition, mPosition + mLookAt, vec3(0.0f, 1.0f, 0.0f) );
}

void FirstPersonCamera::setPosition(vec3 position) {
	mPosition = position;
}

void FirstPersonCamera::setLookAt(vec3 newLookAt) {
	mLookAt = newLookAt;
}
