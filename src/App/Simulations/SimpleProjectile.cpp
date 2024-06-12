#include "App/Simulations/SimpleProjectile.h"

#include "App/Ground.h"
#include "App/CurveManager.h"

#include "imgui.h"

SimpleProjectile::SimpleProjectile(const v2& position, const std::string& type)
	: Simulation(position, type)
{
	intersectXAxis.draw = false;
	intersectXAxis.style = ControlNode::Style::CrossDiagonal;
	intersectXAxis.colour = v4(1.0f);
	intersectXAxis.positionFixed = true;
}

JSONType SimpleProjectile::SaveState()
{
	std::unordered_map<std::string, JSONType> map = {
		{ "dt", dt }
	};

	return { map };
}

void SimpleProjectile::LoadState(JSONType& state)
{
	dt = (float)state.obj["dt"].f;
}

void SimpleProjectile::Calculate()
{
	std::vector<CurveManager::SignificantPoint> sigPoints;

	v2 p0 = startPos.getPosGlobal();
	v2 prevPos = p0;
	v2 vel = startVel.getPosLocal();
	float t = 0.0f;

	bool aboveGround = prevPos.y > 0.0f;
	distanceTravelled = 0.0f;

	std::vector<std::pair<v2, v2>> parabolaData;
	parabolaData.push_back({ prevPos, { } });

	bool ascending = vel.y >= 0.0f;
	for (int i = 0; i < 1000; i++)
	{
		v2 newPos = prevPos + vel * dt;

		if (ascending && newPos.y < prevPos.y)
		{
			ascending = false;
			sigPoints.push_back({ CurveManager::SignificantPoint::Type::Maximum, prevPos, v2(t, (p0 - prevPos).length()) });
		}

		vel = vel - gravity * dt;
		float newt = t + dt;
		float dist = (p0 - newPos).length();
		distanceTravelled += (newPos - prevPos).length();

		if (GetGround().AboveGround(prevPos))
		{
			IntersectionResult r = GetGround().TestIntersect(prevPos, newPos, dt);
			if (r.intersected)
			{
				newPos = r.position;
				newt = t + r.dt;
				dist = (p0 - newPos).length();
				intersectXAxis.draw = true;
				intersectXAxis.setPosGlobal(newPos);

				parabolaData.push_back({ newPos, v2(newt, dist) });
				GetCurveManager().ParabolaData(parabolaData, sigPoints, colour);
				return;
			}
		}

		parabolaData.push_back({ newPos, v2(newt, dist) });
		
		t = newt;
		prevPos = newPos;
	}

	// dispatch to be drawn
	GetCurveManager().ParabolaData(parabolaData, sigPoints, colour);
	intersectXAxis.draw = false;
}

void SimpleProjectile::OnDisable()
{
	startPos.draw = false;
	startVel.draw = false;
	intersectXAxis.draw = false;
}

void SimpleProjectile::OnEnable()
{
	startPos.draw = true;
	startVel.draw = true;
}

void SimpleProjectile::DrawUI()
{
	ImGui::Text(("Distance travelled by projectile: " + ftos(distanceTravelled)).c_str());
	ImGui::SliderFloat("dt", &dt, 0.005f, 10.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
}