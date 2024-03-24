#include "App/Simulations/AnalyticProjectile.h"

#include "App/Ground.h"
#include "Engine/DrawList.h"

#include <sstream>
#include <iomanip>

AnalyticProjectile::AnalyticProjectile()
{
	intersectXAxis.draw = false;
	intersectXAxis.style = ControlNode::Style::CrossDiagonal;
	intersectXAxis.colour = v4(1.0f);
	intersectXAxis.positionFixed = true;

	maximum.style = ControlNode::Style::Cross;
	maximum.colour = v4(1.0f);
	maximum.positionFixed = true;
	maximum.label = "max";
}

void AnalyticProjectile::OnDisable()
{
	startPos.draw = false;
	startVel.draw = false;
	intersectXAxis.draw = false;
	maximum.draw = false;
}

void AnalyticProjectile::OnEnable()
{
	startPos.draw = true;
	startVel.draw = true;
}

void AnalyticProjectile::Draw(DrawList* drawList, AxisType axes)
{
	v2 p0 = startPos.getPosGlobal();
	v2 v0 = startVel.getPosLocal();
	if (GetGround().BelowGround(p0))
	{
		maximum.draw = false;
		intersectXAxis.draw = false;
		return;
	}

	// (x/g)(y+sqrt(y^2+2gh))
	float R = v0.x / gravity.y * (v0.y + sqrtf(v0.y * v0.y + 2.0f * gravity.y * p0.y));
	ParabolaFlag flags = showMaximumDistance ? ParabolaFlag_None : ParabolaFlag_GroundCheck;
	auto result = Parabola(drawList, p0, v0, R, axes, colour, flags);
	
	//if (result.distFromStart.size() >= 2)
	// 	for (size_t i = 0; i < result.distFromStart.size() - 1; i++)
	// 		drawList->Line(result.distFromStart[i].scale(v2(5.0f, 1.0f)), result.distFromStart[i + 1].scale(v2(5.0f, 1.0f)), ImColor(0.4f, 1.0f, 0.2f));

	float RMax = v0.length2() / gravity.y * sqrtf(1.0f + 2.0f * gravity.y * p0.y / v0.length2());
	if (showMaximumDistance)
	{
		// maximum distance
		float thetaMax = asinf(1.0f / sqrtf(2.0f + 2.0f * gravity.y * p0.y / v0.length2()));
		v2 vMax = v2((v0.x < 0.0f ? -1.0f : 1.0f) * cosf(thetaMax), sinf(thetaMax)) * v0.length();
		Parabola(drawList, p0, vMax, (v0.x < 0.0f ? -1.0f : 1.0f) * RMax, axes, colour * 0.6f, ParabolaFlag_None);
	}

	if (showBoundingParabola && axes == AxisType::XY)
	{
		float height = v0.length2() / (2.0f * gravity.y);

		bool first = true;
		v2 np;
		for (float x = -RMax; x < RMax; x = std::min(RMax, x + RMax * 0.01f))
		{
			float y = height - gravity.y / (2.0f * v0.length2()) * x * x;
			v2 p = p0 + v2(x, y);
			if (first)
				first = false;
			else
				drawList->Line(np, p, ImColor(0.4f, 0.8f, 1.0f));
			np = p;
		}
	}

	// maximum point on normal curve
	v2 max = v2(p0.x + v0.x * v0.y / gravity.y, p0.y + v0.y * v0.y / (2.0f * gravity.y));
	if (v0.x > 0.0f && max.x < p0.x || v0.x < 0.0f && max.x > p0.x || v0.y < 0.0f && max.y > p0.y)
		maximum.draw = false;
	else
	{
		maximum.draw = true;
		maximum.setPosGlobal(max);
	}

	// x-intercept
	intersectXAxis.draw = true;
	if (abs(v0.x) <= vyEpsilon)
	{
		v2 end = GetGround().VerticallyNearestTo(p0);
		intersectXAxis.setPosGlobal(end);
	}
	else
		intersectXAxis.setPosGlobal(result.hitPos);
}

void AnalyticProjectile::DrawUI()
{
	startPos.UI(0);
	startVel.UI(10);

	maximum.UI(20, true);
	
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