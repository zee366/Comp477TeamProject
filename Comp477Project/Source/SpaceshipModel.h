#pragma once

#include "Model.h"

class SpaceshipModel : public Model
{
public:
	SpaceshipModel(glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f));
	virtual ~SpaceshipModel();

	virtual void Update(float dt);
	virtual void Draw();

protected:
	virtual bool ParseLine(const std::vector<ci_string> &token);

private:
	bool searching = true;
	glm::vec3 mLookAt;
	int searchDirection = 1;
	int vertexCount;
	Model* targetPrey; 
	unsigned int mVAO;
	unsigned int mVBO[3];
	ci_string chasingMsg;
	ci_string searchingMsg; 
};