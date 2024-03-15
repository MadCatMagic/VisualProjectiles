#pragma once
#include "App/Simulation.h"

class SimpleProjectile : public Simulation
{
public:
	SimpleProjectile();

private:
	float t0 = 0.0f;

	virtual void OnDisable() override;
	virtual void OnEnable() override;

	virtual void Draw(DrawList* drawList, AxisType axes) override;
	virtual void DrawUI() override;

	ControlNode intersectXAxis;
};