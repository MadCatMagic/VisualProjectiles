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

	float projectileDistanceLimit(float z) const;
	float projectileDistance(const v2& v0, float theta, float u);

	std::string ftos(float f) const;

	ControlNode intersectXAxis;
	ControlNode maximum;

	bool showMaximumDistance = false;
	bool showBoundingParabola = false;

	std::vector<v2> vframeData;
	float maxT{};
	float maxDist{};
};