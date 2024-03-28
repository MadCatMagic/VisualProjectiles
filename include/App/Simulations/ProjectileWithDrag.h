#pragma once
#include "App/Simulation.h"

class ProjectileWithDrag : public Simulation
{
public:
	ProjectileWithDrag();

private:
	float distanceTravelled = 0.0f;
	float dt = 0.1f;

	bool usePhysicalObject = false;
	const char* dragNames[9] {
		"Sphere", "HalfSphere", "Cone", "Cube", "AngledCube", "LongCylinder", "ShortCylinder", "StreamlinedBody", "StreamlinedHalfBody"
	};
	enum DragShape {
		Sphere, HalfSphere, Cone, Cube, AngledCube, LongCylinder, ShortCylinder, StreamlinedBody, StreamlinedHalfBody 
	} shape = DragShape::Sphere;
	float dragCoefficient = 0.5f;
	float airDensity = 1.0f;
	float crossSectionalArea = 0.002f;
	float mass = 1.0f;

	virtual void OnDisable() override;
	virtual void OnEnable() override;

	//virtual void Draw(DrawList* drawList, AxisType axes) override;
	virtual void Calculate() override;
	virtual void DrawUI() override;

	ControlNode intersectXAxis;
};