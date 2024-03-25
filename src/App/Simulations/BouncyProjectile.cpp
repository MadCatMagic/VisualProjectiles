#include "App/Simulations/BouncyProjectile.h"
#include "App/Ground.h"

#include "Engine/DrawList.h"

#include "imgui.h"

BouncyProjectile::BouncyProjectile()
{
}

void BouncyProjectile::OnDisable()
{
	startPos.draw = false;
	startVel.draw = false;
}

void BouncyProjectile::OnEnable()
{
	startPos.draw = true;
	startVel.draw = true;
}

void BouncyProjectile::Draw(DrawList* drawList, AxisType axes)
{
	v2 p0 = startPos.getPosGlobal();
	v2 prevPos = p0;
	v2 vel = startVel.getPosLocal();
	float t = 0.0f;

	distanceTravelled = 0.0f;

	if (GetGround().BelowGround(p0))
		return;

	std::vector<std::pair<v2, v2>> parabolaData;
	parabolaData.push_back({ prevPos, { } });

	int escape = 0;
	for (int bounces = 0; bounces <= maxBounces && escape < 2000; escape++)
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
				bounces++;

				newPos = r.position;
				newt = t + r.dt;

				parabolaData.push_back({ newPos, v2(newt, dist) });

				
				vel = vel.reflect(r.normal) * bounceCoeff;
			}
		}

		parabolaData.push_back({ newPos, v2(newt, dist) });

		t = newt;
		prevPos = newPos;
	}

	// dispatch to be drawn
	drawList->ParabolaData(parabolaData, ImColor(colour.x, colour.y, colour.z));
}

void BouncyProjectile::DrawUI()
{
	ImGui::Text(("Distance travelled by projectile: " + ftos(distanceTravelled)).c_str());
	ImGui::SliderFloat("dt", &dt, 0.005f, 10.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("bounce coefficient", &bounceCoeff, 0.0f, 1.0f);
	ImGui::SliderInt("max bounces", &maxBounces, 0, 25);
}
