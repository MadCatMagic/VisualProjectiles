#pragma once
#include "App/Simulation.h"


class TaskOneProjectile : public Simulation
{
public:
	TaskOneProjectile();

private:
	float t0 = 0.0f;

	virtual void OnDisable() override;
	virtual void OnEnable() override;

	virtual void Draw(class DrawList* drawList, AxisType axes) override;
	virtual void DrawUI() override;

	ControlNode intersectXAxis;
};