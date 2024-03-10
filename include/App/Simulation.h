#pragma once
#include "Vector.h"
#include "ControlNode.h"

class Simulation
{
public:
	virtual void Draw(class DrawList* drawList) = 0;
	virtual void DrawUI() = 0;

	// dont bother putting ui for this, it automatically adds them anyway
	std::string name = "Squidward";
	v3 colour = v3::one;

	static v2 gravity;
};

class TaskOneProjectile : public Simulation
{
public:
	TaskOneProjectile();

	virtual void Draw(class DrawList* drawList);
	virtual void DrawUI();

private:
	ControlNode startPos;
	ControlNode startVel{ v2(5.0f), &startPos };
};