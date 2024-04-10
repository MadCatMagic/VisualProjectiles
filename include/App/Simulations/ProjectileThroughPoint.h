#pragma once

#include "App/Simulation.h"

class ProjectileThroughPoint : public Simulation
{
public:
	ProjectileThroughPoint(const v2& position);

private:
	bool drawMaximumPossibilitiesLine = false;
	bool lockMinU = true;
	bool maximumPossibilitiesLineLower = false;
	float r = 10.0f;

	virtual void OnDisable() override;
	virtual void OnEnable() override;

	//virtual void Draw(DrawList* drawList, AxisType axes) override;
	virtual void Calculate() override;
	virtual void DrawUI() override;

	ControlNode controlPoint;
	ControlNode maximum;

	ControlNode shyMaximum;
	ControlVector vel2;
};