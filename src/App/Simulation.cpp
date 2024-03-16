#include "App/Simulation.h"
#include "App/Ground.h"

#include "imgui.h"

#include "Engine/DrawList.h"

v2 Simulation::gravity = v2(0.0f, 9.81f);

Simulation::Simulation()
{
	startVel.label = "v0";
	startPos.label = "p0";
	startPos.style = ControlNode::Style::Circle;
	startPos.colour = v4(1.0f, 0.6f, 0.2f, 0.9f);
	startVel.style = ControlNode::Style::Circle;
	startVel.colour = v4(1.0f, 0.6f, 0.2f, 0.9f);
}

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


std::pair<bool, v2> Simulation::Parabola(DrawList* dl, const v2& p0, const v2& v0, float R, bool groundCheck, AxisType axes, const v4& col)
{
	if (v0.x == 0.0f)
		return std::make_pair(false, v2());

	ImColor imCol = ImColor(col.x, col.y, col.z, col.w);

	v2 pp = p0;
	float pt = 0.0f;
	int i = 0;
	for (float x = p0.x; i <= 200; x += 0.005f * R)
	{
		float tt = v0.y / v0.x;
		float rx = x - p0.x;
		float y = p0.y + rx * tt - gravity.y / (2.0f * v0.length2()) * (1.0f + tt * tt) * rx * rx;
		v2 np = v2(x, y);
		float nt = rx / v0.x;

		if (groundCheck && i > 1)
		{
			auto pair = GetGround().TestIntersect(pp, np);
			if (pair.first)
			{
				np = pair.second;
				nt = (np.x - p0.x) / v0.x;
				dl->Line(splitAxes(pp, pt, axes), splitAxes(np, nt, axes), imCol);
				return std::make_pair(true, np);
			}
		}

		// plot
		dl->Line(splitAxes(pp, pt, axes), splitAxes(np, nt, axes), imCol);

		pp = np;
		pt = nt;
		i++;
	}
	return std::make_pair(false, pp);
}