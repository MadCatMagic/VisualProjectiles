#pragma once
#include "App/Simulation.h"

class SimpleProjectile : public Simulation
{
public:
	SimpleProjectile();

private:
	float distanceTravelled = 0.0f;
	float dt = 0.1f;

	virtual void OnDisable() override;
	virtual void OnEnable() override;

	virtual void Draw(DrawList* drawList, AxisType axes) override;
	virtual void DrawUI() override;

	ControlNode intersectXAxis;
};