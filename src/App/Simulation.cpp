#include "App/Simulation.h"
#include "App/Ground.h"

#include "imgui.h"

#include "App/CurveManager.h"

#include <sstream>
#include <iomanip>

v2 Simulation::gravity = v2(0.0f, 9.81f);

Simulation::Simulation(const v2& position, const std::string& type)
	: type(type)
{
	startVel.label = "v0";
	startPos.label = "p0";
	startPos.style = ControlNode::Style::Circle;
	startPos.colour = v4(1.0f, 0.6f, 0.2f, 0.9f);
	startVel.style = ControlNode::Style::Circle;
	startVel.colour = v4(1.0f, 0.6f, 0.2f, 0.9f);
	startPos.setPosGlobal(position);
}

JSONType Simulation::_SaveState()
{
	std::unordered_map<std::string, JSONType> map = {
		{ "name", name },
		{ "colour", colour },
		{ "enabled", enabled },
		{ "startPos", startPos.SaveState() },
		{ "startVel", startVel.SaveState() },
		{ "sim", SaveState() },
		{ "type", type }
	};
	return { map };
}

void Simulation::_LoadState(JSONType& state)
{
	name = state.obj["name"].s;
	auto& colVec = state.obj["colour"].arr;
	colour = v3((float)colVec[0].f, (float)colVec[1].f, (float)colVec[2].f);
	enabled = state.obj["enabled"].b;
	startPos = ControlNode(state.obj["startPos"]);
	startVel = ControlVector(state.obj["startVel"]);

	LoadState(state.obj["sim"]);
}

std::string Simulation::ftos(float f, int sf) const
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(sf) << f;
	return stream.str();
}

ParabolaResult Simulation::Parabola(const v2& p0, const v2& v0, float R, const v4& col, ParabolaFlag flags)
{
	std::vector<p6> parabolaData;
	//ImColor imCol = ImColor(col.x, col.y, col.z, col.w);

	std::vector<CurveManager::SignificantPoint> sigPoints;

	// maximum point on normal curve
	v2 max = v2(p0.x + v0.x * v0.y / gravity.y, p0.y + v0.y * v0.y / (2.0f * gravity.y));
	v2 maxTD = v2(1.0f / gravity.y * v0.y, (max - p0).length());
	bool includeMax = !(v0.x > 0.0f && max.x < p0.x || v0.x < 0.0f && max.x > p0.x || v0.y < 0.0f && max.y > p0.y);

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
			parabolaData.push_back({ v2(p0.x, y + p0.y), v2(t, abs(y)), v2(0.0f, v0.y - gravity.y * t) });

			if (t >= maxTD.x && includeMax)
			{
				sigPoints.push_back({ CurveManager::SignificantPoint::Type::Maximum, max, maxTD, {} });
				includeMax = false;
			}
		}

		sigPoints.push_back({ CurveManager::SignificantPoint::Type::XIntersect, min, v2(tm, abs(p0.y - min.y)), v2(0.0f, v0.y - gravity.y * tm)});
		GetCurveManager().ParabolaData(parabolaData, sigPoints, col);
		
		ParabolaResult result;
		result.hitGround = flags & ParabolaFlag_GroundCheck;
		result.hitPos = min;

		result.maxT = tm;
		result.maxDist = ym;

		return result;
	}

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

		if (nt >= maxTD.x && includeMax)
		{
			sigPoints.push_back({ CurveManager::SignificantPoint::Type::Maximum, max, maxTD, v2(v0.x, 0.0f) });
			includeMax = false;
		}

		if ((flags & ParabolaFlag_GroundCheck) && i > 1)
		{
			IntersectionResult r = GetGround().TestIntersect(pp, np, 0.0f);
			if (r.intersected)
			{
				np = r.position;
				nt = (np.x - p0.x) / v0.x;

				float dist = (np - p0).length();
				maxDist = std::max(dist, maxDist);
				parabolaData.push_back({ np, v2(nt, dist), v0 - gravity * nt });

				GetCurveManager().ParabolaData(parabolaData, sigPoints, col);
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
		parabolaData.push_back({ np, v2(nt, dist), v0 - gravity * nt });

		pp = np;
		pt = nt;
		i++;
	}
	
	GetCurveManager().ParabolaData(parabolaData, sigPoints, col);

	ParabolaResult result;
	result.maxT = pt;
	result.maxDist = maxDist;
	result.hitPos = pp;
	return result;
}