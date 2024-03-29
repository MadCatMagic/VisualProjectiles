#include "App/Simulation.h"
#include "App/Ground.h"

#include "imgui.h"

#include "App/CurveManager.h"

#include <sstream>
#include <iomanip>

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

std::string Simulation::ftos(float f, int sf) const
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(sf) << f;
	return stream.str();
}

ParabolaResult Simulation::Parabola(const v2& p0, const v2& v0, float R, const v4& col, ParabolaFlag flags)
{
	std::vector<std::pair<v2, v2>> parabolaData;
	parabolaData.push_back({ p0, v2::zero });
	//ImColor imCol = ImColor(col.x, col.y, col.z, col.w);

	if (abs(v0.x) < vyEpsilon)
	{
		// assumes p0.y > ground
		v2 min = (flags & ParabolaFlag_GroundCheck) ? GetGround().VerticallyNearestTo(p0) : v2(p0.x, 0.0f);

		// a = -g; v = -gt + v0; p = -0.5gt^2 + v0t + p0
		// want p = 0
		// 0 = -0.5gt^2 + v0t + p0
		// t = (-v0 +- sqrt(v0*v0 + 2gp0)) / -g
		float tm = (v0.y + sqrt(v0.y * v0.y + 2.0f * gravity.y * p0.y)) / (gravity.y);
		float ym = 0.0f;
		for (float t = 0.0f; t <= tm; t += 0.005f * tm)
		{
			float y = -0.5f * gravity.y * t * t + v0.y * t;
			ym = std::max(abs(y), ym);
			parabolaData.push_back({ v2(p0.x, y), v2(t, abs(y)) });
		}

		GetCurveManager().ParabolaData(parabolaData, col);
		
		ParabolaResult result;
		result.hitGround = flags & ParabolaFlag_GroundCheck;
		result.hitPos = min;

		result.maxT = tm;
		result.maxDist = ym;

		return result;
	}

	std::vector<v2> distAgainstTime;

	v2 pp = p0;
	float pt = 0.0f;
	int i = 0;

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
			IntersectionResult r = GetGround().TestIntersect(pp, np, 0.0f);
			if (r.intersected)
			{
				np = r.position;
				nt = (np.x - p0.x) / v0.x;

				float dist = (np - p0).length();
				maxDist = std::max(dist, maxDist);
				parabolaData.push_back({ np, v2(nt, dist) });

				GetCurveManager().ParabolaData(parabolaData, col);
				ParabolaResult result;
				result.hitGround = true;
				result.hitPos = np;
				result.maxT = nt;
				result.maxDist = maxDist;
				return result;
			}
		}

		// plot
		float dist = (np - p0).length();
		maxDist = std::max(dist, maxDist);
		parabolaData.push_back({ np, v2(nt, dist) });

		pp = np;
		pt = nt;
		i++;
	}
	
	GetCurveManager().ParabolaData(parabolaData, col);

	ParabolaResult result;
	result.maxT = pt;
	result.maxDist = maxDist;
	result.hitPos = pp;
	return result;
}