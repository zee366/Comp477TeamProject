//
// COMP 371 Assignment Framework
//
// Created by Nicolas Bergeron on 15/7/15.
//         with help from Jordan Rutty
//
// Copyright (c) 2014-2019 Concordia University. All rights reserved.
//

#include "ParticleEmitter.h"
#include "Model.h"

using namespace glm;

ParticleEmitter::ParticleEmitter(glm::vec3 position, const Model* parent)
: mpParent(parent), mPosition(position)
{
}

glm::vec3 ParticleEmitter::GetPosition()
{
    // Position from Parented Emitter
    //
    // Return the position where the particle is emitted.
    // If the emitter is parented, the position is relative to its parent

	glm::vec4 position = vec4(mPosition, 1.0f);
	position = mpParent->GetWorldMatrix()*position;

	return vec3(position);
}




