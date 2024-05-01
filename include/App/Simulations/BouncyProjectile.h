#pragma once
#include "App/Simulation.h"

class BouncyProjectile : public Simulation
{
public:
	BouncyProjectile(const v2& position, const std::string& type);

private:
	virtual JSONType SaveState();
	virtual void LoadState(JSONType& state);

	float distanceTravelled = 0.0f;
	float dt = 0.1f;

	float bounceCoeff = 0.4f;
	int maxBounces = 5;

	virtual void OnDisable() override;
	virtual void OnEnable() override;

	//virtual void Draw(DrawList* drawList, AxisType axes) override;
	virtual void Calculate() override;
	virtual void DrawUI() override;

};