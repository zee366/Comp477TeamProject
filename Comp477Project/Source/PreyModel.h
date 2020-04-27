#pragma once

#include "Model.h"

class PreyModel : public Model
{
public:
	PreyModel(glm::vec3 size = glm::vec3(1.0f, 1.0f, 1.0f));
	virtual ~PreyModel();

	void SetChased(bool chased);

	virtual void Update(float dt);
	virtual void Draw();

protected:
	virtual bool ParseLine(const std::vector<ci_string> &token);

private:
	bool chased = false;
	glm::vec3 mLookAt;
	std::vector<bool> reachedStorms;
	int goalStorm = -1;

	int vertexCount;

	unsigned int mVAO;
	unsigned int mVBO[3];
};