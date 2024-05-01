#pragma once

#include "App/Simulation.h"

class ProjectileThroughPoint : public Simulation
{
public:
	ProjectileThroughPoint(const v2& position, const std::string& type);

private:
	virtual JSONType SaveState();
	virtual void LoadState(JSONType& state);

	bool drawMaximumPossibilitiesLine = false;
	bool lockMinU = true;
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