//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 8/7/14.
// Updated by Gary Chang on 14/1/15
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "Animation.h"
#include "Renderer.h"
#include "World.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace glm;
using namespace std;

AnimationKey::AnimationKey() : Model()
{
}

AnimationKey::~AnimationKey()
{
}

void AnimationKey::Update(float dt)
{
	Model::Update(dt);
}

void AnimationKey::Draw()
{
	Model::Draw();
}

bool AnimationKey::ParseLine(const std::vector<ci_string> &token)
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

Animation::Animation() 
	: mName(""), mCurrentTime(0.0f), mDuration(0.0f), mVBO(0), mVAO(0),mAnimationSpeed(1.0f)
{
}

Animation::~Animation()
{
}

void Animation::CreateVertexBuffer()
{
	for (int i = 0; i < mKey.size() - 1; ++i)
	{
		spline.AddControlPoint(mKey[i].GetPosition());
	}

	spline.CreateVertexBuffer();

}

void Animation::Update(float dt)
{
	mCurrentTime += dt;

    while(mCurrentTime > mDuration)
    {
        mCurrentTime -= mDuration;
    }
}

void Animation::Draw()
{
	spline.Draw();
}


void Animation::Load(ci_istringstream& iss)
{
	ci_string line;

	// Parse model line by line
	while(std::getline(iss, line))
	{
		// Splitting line into tokens
		ci_istringstream strstr(line);
		istream_iterator<ci_string, char, ci_char_traits> it(strstr);
		istream_iterator<ci_string, char, ci_char_traits> end;
		vector<ci_string> token(it, end);

		if (ParseLine(token) == false)
		{
			fprintf(stderr, "Error loading scene file... token:  %s!", token[0].c_str());
			getchar();
			exit(-1);
		}
	}
}


bool Animation::ParseLine(const std::vector<ci_string> &token)
{
    if (token.empty())
    {
        return true;
    }
	else if (token[0] == "name")
	{
		assert(token.size() > 2);
		assert(token[1] == "=");

		mName = token[2];
		return true;
	}
	else if (token[0] == "key")
	{
		assert(token.size() > 5);
        assert(token[1] == "=");
        assert(token[3] == "time");
        assert(token[4] == "=");

		ci_string name = token[2];
		AnimationKey *key = World::GetInstance()->FindAnimationKey(name);
        
		assert(key != nullptr);
		AddKey(key, (float) atof(token[5].c_str()));
		return true;
	}
	else if (token[0] == "animationSpeed")
	{
		assert(token.size() > 2);
		assert(token[1] == "=");

		mAnimationSpeed  = static_cast<float>(atof(token[2].c_str()));
		return true;
	}
	return false;
}

void Animation::AddKey(AnimationKey* key, float time)
{
    assert(time >= mDuration);
	mKey.push_back(*key);
    mKeyTime.push_back(time);
    mDuration = time;
}

ci_string Animation::GetName() const
{
	return mName;
}

glm::mat4 Animation::GetAnimationWorldMatrix() const
{
	// Find the 2 keys to interpolate the transformation (before and after current time)
	// Interpolate the position, scaling and rotation separately
	// Finally concatenate the interpolated transforms into a single
	// world transform and return it.

	int key1 = 0, key2 = 0;
	float normalizedTime = 0.0f;

	for (int i = 1; i < (int) mKey.size(); i++)
	{
		if (mCurrentTime < mKeyTime[i])
		{
			key1 = i - 1;
			key2 = i;
			normalizedTime = (mCurrentTime - mKeyTime[key1]) / (mKeyTime[key2] - mKeyTime[key1]);
		}
	}

	vec3 position = spline.GetPosition(key1 + normalizedTime);

	quat q1 = glm::angleAxis(radians(mKey[key1].GetRotationAngle()), mKey[key1].GetRotationAxis());
	quat q2 = glm::angleAxis(radians(mKey[key2].GetRotationAngle()), mKey[key2].GetRotationAxis());
	quat q3 = glm::slerp(q1, q2, normalizedTime * mAnimationSpeed);

	glm::mat4 rotationMatrix = glm::toMat4(q3);
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

	return translationMatrix * rotationMatrix;
}

glm::vec3 Animation::getposition()
{
	int key1 = 0, key2 = 0;
	float normalizedTime = 0.0f;

	for (int i = 1; i < (int)mKey.size(); i++)
	{
		if (mCurrentTime < mKeyTime[i])
		{
			key1 = i - 1;
			key2 = i;
			normalizedTime = (mCurrentTime - mKeyTime[key1]) / (mKeyTime[key2] - mKeyTime[key1]);
		}
	}

	return spline.GetPosition(key1 + normalizedTime);
}
