#pragma once
#include "Vector.h"
#include "ControlNode.h"

enum AxisType {
	XY, XT, YT
};

class Simulation
{
public:
	friend class App;

	virtual void Draw(DrawList* drawList, AxisType axes) = 0;
	virtual void DrawUI() = 0;

	// dont bother putting ui for this, it automatically adds them anyway
	std::string name = "Squidward";
	v3 colour = v3::one;

	static v2 gravity;

	bool enabled = true;

	virtual void OnDisable() = 0;
	virtual void OnEnable() = 0;

protected:
	v2 splitAxes(float x, float y, float t, AxisType type);
	v2 splitAxes(const v2& p, float t, AxisType type);

	ControlNode startPos;
	ControlVector startVel{ v2(5.0f), &startPos };
};