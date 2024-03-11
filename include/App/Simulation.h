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

	virtual void Draw(class DrawList* drawList, AxisType axes) = 0;
	virtual void DrawUI() = 0;

	// dont bother putting ui for this, it automatically adds them anyway
	std::string name = "Squidward";
	v3 colour = v3::one;

	static v2 gravity;

protected:
	ControlNode startPos;
	ControlVector startVel{ v2(5.0f), &startPos };
};

class TaskOneProjectile : public Simulation
{
public:
	TaskOneProjectile();

	virtual void Draw(class DrawList* drawList, AxisType axes);
	virtual void DrawUI();

private:
	float t0 = 0.0f;
};