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


ParabolaResult Simulation::Parabola(DrawList* dl, const v2& p0, const v2& v0, float R, AxisType axes, const v4& col, ParabolaFlag flags)
{
	ImColor imCol = ImColor(col.x, col.y, col.z, col.w);
	if (abs(v0.x) < vyEpsilon)
	{
		// assumes p0.y > ground
		v2 min = (flags & ParabolaFlag_GroundCheck) ? GetGround().VerticallyNearestTo(p0) : v2(p0.x, 0.0f);
		float tm = (v0.y + sqrt(v0.y * v0.y + 2.0f * gravity.y * p0.y)) / (gravity.y);
		
		// needs to be drawn differently as per the axes
		if (axes == AxisType::XY)
		{
			if (v0.y > 0.0f)
			{
				v2 max = p0 + v2(v0.x * v0.y / gravity.y, v0.y * v0.y / (2.0f * gravity.y));
				dl->Line(p0, max, imCol);
				dl->Line(max, min, imCol);
			}
			else
				dl->Line(p0, min, imCol);
		}
		else if (axes == AxisType::XT)
			dl->Line(v2(0.0f, p0.x), v2(tm, p0.x), imCol);
		// kinda cheaky
		else if (axes == AxisType::YT)
			Parabola(dl, v2(0.0f, p0.y), v2(1.0f, v0.y), tm, AxisType::XY, col, ParabolaFlag_None);

		ParabolaResult result;
		result.hitGround = flags & ParabolaFlag_GroundCheck;
		result.hitPos = min;
		
		// a = -g; v = -gt + v0; p = -0.5gt^2 + v0t + p0
		// want p = 0
		// 0 = -0.5gt^2 + v0t + p0
		// t = (-v0 +- sqrt(v0*v0 + 2gp0)) / -g
		if (flags & ParabolaFlag_LogDistFromStart)
		{
			float ym = 0.0f;
			for (float t = 0.0f; t <= tm; t += 0.005f * tm)
			{
				float y = -0.5f * gravity.y * t * t + v0.y * t;
				ym = std::max(abs(y), ym);
				result.distFromStart.push_back(v2(t, abs(y)));
			}
			result.maxT = tm;
			result.maxDist = ym;
		}

		return result;
	}

	std::vector<v2> distAgainstTime;

	v2 pp = p0;
	float pt = 0.0f;
	int i = 0;

	if (flags & ParabolaFlag_LogDistFromStart)
		distAgainstTime.push_back(v2(pt, (pp - p0).length()));

	float maxDist = 0.0f;

	for (float x = p0.x; i <= 200; x += 0.005f * R)
	{
		float tt = v0.y / v0.x;
		float rx = x - p0.x;
		float y = p0.y + rx * tt - gravity.y / (2.0f * v0.length2()) * (1.0f + tt * tt) * rx * rx;
		v2 np = v2(x, y);
		float nt = rx / v0.x;

		if ((flags & ParabolaFlag_GroundCheck) && i > 1)
		{
			auto pair = GetGround().TestIntersect(pp, np);
			if (pair.first)
			{
				np = pair.second;
				nt = (np.x - p0.x) / v0.x;

				if (flags & ParabolaFlag_LogDistFromStart)
				{
					float dist = (np - p0).length();
					maxDist = std::max(dist, maxDist);
					distAgainstTime.push_back(v2(nt, dist));
				}

				dl->Line(splitAxes(pp, pt, axes), splitAxes(np, nt, axes), imCol);
				ParabolaResult result;
				result.hitGround = true;
				result.hitPos = np;
				result.distFromStart = distAgainstTime;
				result.maxT = nt;
				result.maxDist = maxDist;
				return result;
			}
		}

		if (flags & ParabolaFlag_LogDistFromStart)
		{
			float dist = (np - p0).length();
			maxDist = std::max(dist, maxDist);
			distAgainstTime.push_back(v2(nt, dist));
		}

		// plot
		dl->Line(splitAxes(pp, pt, axes), splitAxes(np, nt, axes), imCol);

		pp = np;
		pt = nt;
		i++;
	}

	ParabolaResult result;
	result.distFromStart = distAgainstTime;
	result.maxT = pt;
	result.maxDist = maxDist;
	result.hitPos = pp;
	return result;
}