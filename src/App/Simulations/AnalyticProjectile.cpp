#include "App/Simulations/AnalyticProjectile.h"

#include "App/Ground.h"
#include "Engine/DrawList.h"

AnalyticProjectile::AnalyticProjectile()
{
	intersectXAxis.draw = false;
	intersectXAxis.style = ControlNode::Style::CrossDiagonal;
	intersectXAxis.colour = v4(1.0f);
	intersectXAxis.positionFixed = true;

	maximum.style = ControlNode::Style::Cross;
	maximum.colour = v4(1.0f);
	maximum.positionFixed = true;
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
	if (GetGround().BelowGround(p0) || v0.x <= 0.0f)
	{
		maximum.draw = false;
		intersectXAxis.draw = false;
		return;
	}

	// (x/g)(y+sqrt(y^2+2gh))
	float R = v0.x / gravity.y * (v0.y + sqrtf(v0.y * v0.y + 2.0f * gravity.y * p0.y));
	auto pair = Parabola(drawList, p0, v0, R, !showMaximumDistance, axes, colour);

	if (showMaximumDistance)
	{
		// maximum distance
		float thetaMax = asinf(1.0f / sqrtf(2.0f + 2.0f * gravity.y * p0.y / v0.length2()));
		float RMax = v0.length2() / gravity.y * sqrtf(1.0f + 2.0f * gravity.y * p0.y / v0.length2());
		v2 vMax = v2(cosf(thetaMax), sinf(thetaMax)) * v0.length();
		Parabola(drawList, p0, vMax, RMax, false, axes, colour * 0.6f);
	}

	// maximum point on normal curve
	v2 max = v2(p0.x + v0.x * v0.y / gravity.y, p0.y + v0.y * v0.y / (2.0f * gravity.y));
	if (max.x < p0.x)
		maximum.draw = false;
	else
	{
		maximum.draw = true;
		maximum.setPosGlobal(max);
	}

	// x-intercept
	intersectXAxis.draw = true;
	intersectXAxis.setPosGlobal(pair.second);
}

void AnalyticProjectile::DrawUI()
{
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
}