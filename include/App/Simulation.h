#pragma once
#include "Vector.h"
#include "ControlNode.h"

enum AxisType {
	XY, XT, YT
};

typedef unsigned int ParabolaFlag;
#define ParabolaFlag_None 0u
#define ParabolaFlag_GroundCheck 1u
#define ParabolaFlag_LogDistFromStart 2u

struct ParabolaResult 
{
	inline ParabolaResult() : hitGround(false) { };
	bool hitGround;
	v2 hitPos;

	// only filled if it is set in the function to do so
	std::vector<v2> distFromStart;
	float maxT;
	float maxDist;
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
	const float vyEpsilon = 0.00001f;
	v2 splitAxes(float x, float y, float t, AxisType type);
	v2 splitAxes(const v2& p, float t, AxisType type);

	ControlNode startPos;
	ControlVector startVel{ v2(5.0f), &startPos };

	// <collided with ground, collision point if collided>
	
	ParabolaResult Parabola(DrawList* dl, const v2& p0, const v2& v0, float R, AxisType axes, const v4& col, ParabolaFlag flags);
};