#pragma once
#include "Vector.h"
#include "ControlNode.h"

enum AxisType {
	XY, XT, YT
};

class Simulation
{
public:
	Simulation();
	inline virtual ~Simulation() { }

	friend class App;
	friend class SimulationFactory;

	virtual void Draw(class DrawList* drawList, AxisType axes) = 0;
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

	// <collided with ground, collision point if collided>
	std::pair<bool, v2> Parabola(DrawList* dl, const v2& p0, const v2& v0, float R, bool groundCheck, AxisType axes, const v4& col);
};