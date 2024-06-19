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
	v2 p0 = startPos.getPosGlobal();
	v2 prevPos = p0;
	v2 vel = startVel.getPosLocal();
	float t = 0.0f;

	distanceTravelled = 0.0f;

	std::vector<p6> parabolaData;
	parabolaData.push_back({ prevPos, { }, vel });

	v2 lastVel = vel;

	int maxIters = !GetGround().BelowGround(prevPos) ? 500000 : 10000;
	for (int i = 0; i < maxIters; i++)
	{
		v2 newPos = prevPos + vel * dt;

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

				parabolaData.push_back({ newPos, v2(newt, dist), vel + gravity * (dt - r.dt) });
				GetCurveManager().ParabolaData(parabolaData, {}, colour, true);
				return;
			}
		}

		// GENIUS
		if (vel.angleTo(lastVel) * RAD_TO_DEG > 0.5f)
		{
			lastVel = vel;
			parabolaData.push_back({ newPos, v2(newt, dist), vel });
		}
		
		t = newt;
		prevPos = newPos;
	}

	// dispatch to be drawn
	GetCurveManager().ParabolaData(parabolaData, {}, colour, true);
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
	ImGui::SliderFloat("dt", &dt, 0.0001f, 10.0f, "%.4f", ImGuiSliderFlags_Logarithmic);

	startPos.UI(0);
	startVel.UI(1);
}