#pragma once
#include "App/Simulation.h"


class TaskOneProjectile : public Simulation
{
public:
	TaskOneProjectile();

private:
	float t0 = 0.0f;

	virtual void OnDisable() override;
	virtual void OnEnable() override;

	virtual void Draw(DrawList* drawList, AxisType axes) override;
	virtual void DrawUI() override;

	ControlNode intersectXAxis;
};

class TaskTwoProjectile : public Simulation
{
public:
	TaskTwoProjectile();

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