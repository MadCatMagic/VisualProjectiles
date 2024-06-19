#include "App/Simulations/BouncyProjectile.h"
#include "App/Ground.h"

#include "App/CurveManager.h"

#include "imgui.h"

BouncyProjectile::BouncyProjectile(const v2& position, const std::string& type)
	: Simulation(position, type)
{
}

JSONType BouncyProjectile::SaveState()
{
	std::unordered_map<std::string, JSONType> map = {
		{ "dt", dt },
		{ "bounceCoeff", bounceCoeff },
		{ "maxBounces", (long)maxBounces }
	};

	return { map };
}

void BouncyProjectile::LoadState(JSONType& state)
{
	dt = (float)state.obj["dt"].f;
	bounceCoeff = (float)state.obj["bounceCoeff"].f;
	maxBounces = (int)state.obj["maxBounces"].i;
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

void BouncyProjectile::Calculate()
{
	v2 p0 = startPos.getPosGlobal();
	v2 prevPos = p0;
	v2 vel = startVel.getPosLocal();
	float t = 0.0f;

	distanceTravelled = 0.0f;

	if (GetGround().BelowGround(p0))
		return;

	std::vector<p6> parabolaData;
	std::vector<CurveManager::StaticLine> staticLineData;

	v2 lastVel = vel;

	parabolaData.push_back({ prevPos, { }, vel });

	int escape = 0;
	int maxIters = !GetGround().BelowGround(prevPos) ? 500000 : 10000;
	for (int bounces = 0; bounces <= maxBounces && escape < maxIters; escape++)
	{
		v2 newPos = prevPos + vel * dt;
		float newt = t + dt;
		float dist = (p0 - newPos).length();

		vel -= gravity * dt;

		if (GetGround().AboveGround(prevPos))
		{
			IntersectionResult r = GetGround().TestIntersect(prevPos, newPos, dt);
			if (r.intersected)
			{
				bounces++;

				newPos = r.position;
				newt = t + r.dt;
				dist = (p0 - newPos).length();
				
				vel += gravity * (dt - r.dt);
				vel = vel.reflect(r.normal) * bounceCoeff;

				staticLineData.push_back({
					newPos,
					newPos + r.normal * vel.length(),
					newt
				});
			}
		}

		distanceTravelled += (newPos - prevPos).length();

		// TODO need to move all of these into the rendering code itself as it cannot work out the best place for maxima and stuff otherwise
		if (vel.angleTo(lastVel) * RAD_TO_DEG > 1.0f)
		{
			lastVel = vel;
			parabolaData.push_back({ newPos, v2(newt, dist), vel });
		}

		t = newt;
		prevPos = newPos;
	}

	// dispatch to be drawn
	GetCurveManager().ParabolaData(parabolaData, std::vector<CurveManager::SignificantPoint>(), colour, true);
	//GetCurveManager().StaticLineXYData(staticLineData, v4(0.0f, 0.6f, 1.0f));
}

void BouncyProjectile::DrawUI()
{
	ImGui::Text(("Distance travelled by projectile: " + ftos(distanceTravelled)).c_str());
	ImGui::SliderFloat("dt", &dt, 0.0001f, 10.0f, "%.4f", ImGuiSliderFlags_Logarithmic);
	ImGui::SliderFloat("bounce coefficient", &bounceCoeff, 0.0f, 1.0f);
	ImGui::SliderInt("max bounces", &maxBounces, 0, 50, "%d", ImGuiSliderFlags_Logarithmic);

	startPos.UI(0);
	startVel.UI(1);
}
