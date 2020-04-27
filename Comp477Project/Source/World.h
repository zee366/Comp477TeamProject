//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#pragma once


#include "ParsingHelper.h"
#include "Billboard.h"
#include <vector>

class Camera;
class Model;
class Animation;
class AnimationKey;
class ParticleSystem;
class ParticleDescriptor;
class BSpline;
class AsteroidModel;
class UI_elements;

class World
{
public:
	World();
	~World();
	
    static World* GetInstance();

	void WriteToFile(const char* text);

	void Update(float dt);
	void Draw(float dt);

	void LoadScene(const char * scene_path);
    Animation* FindAnimation(ci_string animName);
    AnimationKey* FindAnimationKey(ci_string keyName);
    ParticleDescriptor* FindParticleDescriptor(ci_string name);
	Model* FindParentModel(ci_string name);

    const Camera* GetCurrentCamera() const;
    void AddBillboard(Billboard* b, ci_string);
    void RemoveBillboard(Billboard* b, ci_string);
    void AddParticleSystem(ParticleSystem* particleSystem);
    void RemoveParticleSystem(ParticleSystem* particleSystem);
    void AddParticleDescriptor(ParticleDescriptor* particleDescriptor);
	bool raycasting(glm::vec3 source, glm::vec3 direction, glm::vec3 prey);
	glm::mat4 collisionCorrection(glm::vec3 source, glm::vec3 direction);
	glm::vec3 getClosestModel(glm::vec3 source);

	std::vector<Model*> getPreyList();
	std::vector<Model*> getCubeList();

private:
    static World* instance;

	std::ofstream mOut;
	std::vector<Model*> mModel;
	std::vector<UI_elements*> mUI;
    std::vector<Animation*> mAnimation;
    std::vector<AnimationKey*> mAnimationKey;
	std::vector<Camera*> mCamera;
	std::vector<BSpline*> mSpline;
    std::vector<ParticleSystem*> mParticleSystemList;
    std::vector<ParticleDescriptor*> mParticleDescriptorList;
	unsigned int mCurrentCamera;
	unsigned int mFirstPersonCameraTarget;
	unsigned int mThirdPersonCameraTarget;
	unsigned int mNumberOfHunters;
	unsigned int mNumberOfPrey;
	int buttonState = 1;
	double speed = 1;

    BillboardList* mpBillboardListFire;
	BillboardList* mpBillboardListSmoke;
	BillboardList* mpBillboardListStorm;
};
