#pragma once
#include "App/Simulation.h"

class AnalyticProjectile : public Simulation
{
public:
	AnalyticProjectile(const v2& position, const std::string& type);

private:
	virtual JSONType SaveState();
	virtual void LoadState(JSONType& state);

	float r = 10.0f;

	virtual void OnDisable() override;
	virtual void OnEnable() override;

	//virtual void Draw(DrawList* drawList, AxisType axes) override;
	virtual void Calculate() override;
	virtual void DrawUI() override;

	float projectileDistanceLimit(float z) const;
	float projectileDistance(const v2& v0, float theta, float u);

	bool showMaximumDistance = false;
	bool showBoundingParabola = false;
};