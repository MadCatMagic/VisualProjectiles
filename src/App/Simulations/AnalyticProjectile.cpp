#include "App/Simulations/AnalyticProjectile.h"

#include "App/Ground.h"
#include "App/CurveManager.h"

#include "imgui.h"

#include <sstream>
#include <iomanip>

AnalyticProjectile::AnalyticProjectile(const v2& position, const std::string& type)
	: Simulation(position, type)
{
}

JSONType AnalyticProjectile::SaveState()
{
	std::unordered_map<std::string, JSONType> map = {
		{ "showMaxDistance", showMaximumDistance },
		{ "showBoundingParabola", showBoundingParabola }
	};

	return { map };
}

void AnalyticProjectile::LoadState(JSONType& state)
{
	showMaximumDistance = state.obj["showMaxDistance"].b;
	showBoundingParabola = state.obj["showBoundingParabola"].b;
}

void AnalyticProjectile::OnDisable()
{
	startPos.draw = false;
	startVel.draw = false;
}

void AnalyticProjectile::OnEnable()
{
	startPos.draw = true;
	startVel.draw = true;
}

void AnalyticProjectile::Calculate()
{
	v2 p0 = startPos.getPosGlobal();
	v2 v0 = startVel.getPosLocal();
	if (GetGround().BelowGround(p0))
		return;

	// (x/g)(y+sqrt(y^2+2gh))
	float R = v0.x / gravity.y * (v0.y + sqrtf(v0.y * v0.y + 2.0f * gravity.y * p0.y));
	ParabolaFlag flags = showMaximumDistance ? ParabolaFlag_None : ParabolaFlag_GroundCheck;
	auto result = Parabola(p0, v0, R, colour, flags);
	
	//if (result.distFromStart.size() >= 2)
	// 	for (size_t i = 0; i < result.distFromStart.size() - 1; i++)
	// 		drawList->Line(result.distFromStart[i].scale(v2(5.0f, 1.0f)), result.distFromStart[i + 1].scale(v2(5.0f, 1.0f)), ImColor(0.4f, 1.0f, 0.2f));

	float RMax = v0.length2() / gravity.y * sqrtf(1.0f + 2.0f * gravity.y * p0.y / v0.length2());
	if (showMaximumDistance)
	{
		// maximum distance
		float thetaMax = asinf(1.0f / sqrtf(2.0f + 2.0f * gravity.y * p0.y / v0.length2()));
		v2 vMax = v2((v0.x < 0.0f ? -1.0f : 1.0f) * cosf(thetaMax), sinf(thetaMax)) * v0.length();
		Parabola(p0, vMax, (v0.x < 0.0f ? -1.0f : 1.0f) * RMax, colour * 0.6f, ParabolaFlag_None);
	}

	if (showBoundingParabola)
	{
		std::vector<v2> drawArr;

		float height = v0.length2() / (2.0f * gravity.y);
		for (float x = -RMax; x < RMax; x = std::min(RMax, x + RMax * 0.01f))
		{
			float y = height - gravity.y / (2.0f * v0.length2()) * x * x;
			v2 p = p0 + v2(x, y);
			drawArr.push_back(p);
		}

		GetCurveManager().CurveXYData(drawArr, v4(0.4f, 0.8f, 1.0f));
	}
}

void AnalyticProjectile::DrawUI()
{
	startPos.UI(0);
	startVel.UI(10);
	
	// calculate projectile distance travelled
	v2 polar = startVel.getPolar();
	float s = projectileDistance(startVel.getPosLocal(), polar.x, polar.y);

	ImGui::Text(("Distance travelled by projectile: " + ftos(s)).c_str());

	ImGui::Checkbox("Show maximum distance", &showMaximumDistance);
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted("This option disables the Ground environment, and just assumes the ground is the line y=0, as otherwise this optimisation becomes far more difficult.");
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
	if (showMaximumDistance)
	{
		v2 p0 = startPos.getPosGlobal();
		v2 v0 = startVel.getPosLocal();
		float thetaMax = asinf(1.0f / sqrtf(2.0f + 2.0f * gravity.y * p0.y / v0.length2()));
		v2 vMax = v2(cosf(thetaMax), sinf(thetaMax)) * v0.length();
		s = projectileDistance(vMax, thetaMax, v0.length());
		ImGui::Text(("Distance travelled by maximised projectile: " + ftos(s)).c_str());
	}
	ImGui::Checkbox("Show bounding parabola", &showBoundingParabola);
}

float AnalyticProjectile::projectileDistanceLimit(float z) const
{
	return 0.5f * (logf(abs(sqrtf(1.0f + z * z) + z)) + z * sqrtf(1.0f + z * z));
}

float AnalyticProjectile::projectileDistance(const v2& v0, float theta, float u)
{
	v2 p0 = startPos.getPosGlobal();
	if (abs(v0.x) < vyEpsilon)
	{
		if (v0.y <= 0.0f)
			return p0.y;

		return p0.y + v0.y * v0.y / gravity.y;
	}
	else
	{
		// (theta, u)
		float R = v0.x / gravity.y * (v0.y + sqrtf(v0.y * v0.y + 2.0f * gravity.y * p0.y));

		float a = tanf(theta);
		float b = a - gravity.y * R * (1.0f + a * a) / (u * u);
		float k = u * u / (gravity.y * (1.0f + a * a));
		return abs(k * (projectileDistanceLimit(a) - projectileDistanceLimit(b)));
	}
}