#include "App/Simulation.h"
#include "App/Ground.h"

#include "imgui.h"

#include "Engine/DrawList.h"

v2 Simulation::gravity = v2(0.0f, -9.81f);

v2 Simulation::splitAxes(float x, float y, float t, AxisType type)
{
	if (type == AxisType::XT)
		return v2(t, x);
	if (type == AxisType::YT)
		return v2(t, y);
	if (type == AxisType::XY)
		return v2(x, y);
	return v2(FLT_MAX, FLT_MAX);
}

v2 Simulation::splitAxes(const v2& p, float t, AxisType type)
{
	return splitAxes(p.x, p.y, t, type);
}
