#pragma once

#include "App/Simulation.h"

class ProjectileThroughPoint : public Simulation
{
public:
	ProjectileThroughPoint();

private:
	float r = 10.0f;

	virtual void OnDisable() override;
	virtual void OnEnable() override;

	virtual void Draw(DrawList* drawList, AxisType axes) override;
	virtual void DrawUI() override;

	ControlNode controlPoint;
	ControlNode maximum;

	ControlVector vel2;
};