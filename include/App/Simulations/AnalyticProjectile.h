#pragma once
#include "App/Simulation.h"

class AnalyticProjectile : public Simulation
{
public:
	AnalyticProjectile();

private:
	float r = 10.0f;

	virtual void OnDisable() override;
	virtual void OnEnable() override;

	virtual void Draw(DrawList* drawList, AxisType axes) override;
	virtual void DrawUI() override;

	ControlNode intersectXAxis;
	ControlNode maximum;

	bool showMaximumDistance = false;
};