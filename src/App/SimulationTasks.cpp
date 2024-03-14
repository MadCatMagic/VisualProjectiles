#include "App/SimulationTasks.h"
#include "App/Ground.h"

#include "Engine/DrawList.h"

#include "imgui.h"

TaskOneProjectile::TaskOneProjectile()
{
	intersectXAxis.draw = false;
	intersectXAxis.style = ControlNode::Style::CrossDiagonal;
	intersectXAxis.colour = v4(1.0f);
	intersectXAxis.positionFixed = true;
}

void TaskOneProjectile::Draw(DrawList* drawList, AxisType axes)
{
	v2 prevPos = startPos.getPosGlobal();
	v2 vel = startVel.getPosLocal();
	float dt = 0.02f;
	float t = t0;

	bool aboveGround = prevPos.y > 0.0f;

	for (int i = 0; i < 1000; i++)
	{

		v2 newPos = prevPos + vel * dt;
		vel = vel + gravity * dt;
		float newt = t + dt;

		if (GetGround().AboveGround(prevPos))
		{
			auto pair = GetGround().TestIntersect(prevPos, newPos);
			if (pair.first)
			{
				newPos = pair.second;
				intersectXAxis.draw = true;
				intersectXAxis.setPosGlobal(newPos);

				drawList->Line(splitAxes(prevPos, t, axes), splitAxes(newPos, newt, axes), ImColor(colour.x, colour.y, colour.z));
				return;
			}
		}

		drawList->Line(splitAxes(prevPos, t, axes), splitAxes(newPos, newt, axes), ImColor(colour.x, colour.y, colour.z));

		t = newt;
		prevPos = newPos;
	}
	intersectXAxis.draw = false;
}

void TaskOneProjectile::OnDisable()
{
	startPos.draw = false;
	startVel.draw = false;
	intersectXAxis.draw = false;
}

void TaskOneProjectile::OnEnable()
{
	startPos.draw = true;
	startVel.draw = true;
}

void TaskOneProjectile::DrawUI()
{
	ImGui::DragFloat("t0", &t0);
}

TaskTwoProjectile::TaskTwoProjectile()
{
	intersectXAxis.draw = false;
	intersectXAxis.style = ControlNode::Style::CrossDiagonal;
	intersectXAxis.colour = v4(1.0f);
	intersectXAxis.positionFixed = true;

	maximum.style = ControlNode::Style::Cross;
	maximum.colour = v4(1.0f);
	maximum.positionFixed = true;
}

void TaskTwoProjectile::OnDisable()
{
	startPos.draw = false;
	startVel.draw = false;
	intersectXAxis.draw = false;
	maximum.draw = false;
}

void TaskTwoProjectile::OnEnable()
{
	startPos.draw = true;
	startVel.draw = true;
}

void TaskTwoProjectile::Draw(DrawList* drawList, AxisType axes)
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
	float R = v0.x / (-gravity.y) * (v0.y + sqrtf(v0.y * v0.y + 2.0f * (-gravity.y) * p0.y));
	auto pair = Parabola(drawList, p0, v0, R, !showMaximumDistance, axes, colour);

	if (showMaximumDistance)
	{
		// maximum distance
		float thetaMax = asinf(1.0f / sqrtf(2.0f - 2.0f * gravity.y * p0.y / v0.length2()));
		float RMax = v0.length2() / (-gravity.y) * sqrtf(1.0f - 2.0f * gravity.y * p0.y / v0.length2());
		v2 vMax = v2(cosf(thetaMax), sinf(thetaMax)) * v0.length();
		Parabola(drawList, p0, vMax, RMax, false, axes, colour * 0.6f);
	}

	// maximum point on normal curve
	v2 max = v2(p0.x - v0.x * v0.y / gravity.y, p0.y - v0.y * v0.y / (2.0f * gravity.y));
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

void TaskTwoProjectile::DrawUI()
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

ProjectileThroughPoint::ProjectileThroughPoint()
{
	controlPoint.style = ControlNode::Style::Circross;
	controlPoint.colour = v4(1.0f, 0.6f, 0.2f, 0.9f);

	startVel.positionFixed = true;
	startVel.colour = v4(0.0f, 0.0f, 0.0f, 0.0f);
	vel2.positionFixed = true;
	vel2.colour = v4(0.0f, 0.0f, 0.0f, 0.0f);
	vel2.root = &startPos;

	maximum.style = ControlNode::Style::Cross;
	maximum.colour = v4(1.0f);
}

void ProjectileThroughPoint::OnDisable()
{
	vel2.draw = false;
	startPos.draw = false;
	startVel.draw = false;
	controlPoint.draw = false;
	maximum.draw = false;
}

void ProjectileThroughPoint::OnEnable()
{
	vel2.draw = true;
	startPos.draw = true;
	startVel.draw = true;
	controlPoint.draw = true;
}

void ProjectileThroughPoint::Draw(DrawList* drawList, AxisType axes)
{
	v2 p0 = startPos.getPosGlobal();
	v2 p1 = controlPoint.getPosGlobal();

	if (GetGround().BelowGround(p0))
	{
		maximum.draw = false;
		return;
	}

	// work out the possible u based on our maximum
	v2 max = maximum.getPosGlobal();
	float k = 2.0f * (max.y - p0.y);
	float u = sqrtf((-gravity.y) * ((max.x - p0.x) * (max.x - p0.x) / k + k));

	// compare it to minimum, reject if too low
	v2 pd = p1 - p0;
	float mag = sqrtf((-gravity.y) * (pd.y + pd.length()));
	if (u >= mag)
		mag = u;

	// calculate velocity vector
	float theta1, theta2;
	{
		float a = (-gravity.y) / (2.0f * mag * mag) * pd.x * pd.x;
		float b = -pd.x;
		float c = pd.y + (-gravity.y) * pd.x * pd.x / (2.0f * mag * mag);
		theta1 = atanf((-b + sqrtf(std::max(0.0f, b * b - 4.0f * a * c))) / (2.0f * a));
		theta2 = atanf((-b - sqrtf(std::max(0.0f, b * b - 4.0f * a * c))) / (2.0f * a));
	}
	v2 v0 = v2(cosf(theta1), sinf(theta1)) * mag;
	v2 v01 = v2(cosf(theta2), sinf(theta2)) * mag;
	startVel.setPosLocal(v0);
	vel2.setPosLocal(v01);

	// (x/g)(y+sqrt(y^2+2gh))
	float R = v0.x / (-gravity.y) * (v0.y + sqrtf(v0.y * v0.y + 2.0f * (-gravity.y) * p0.y));
	auto pair = Parabola(drawList, p0, v0, R, true, axes, colour);

	float R2 = v01.x / (-gravity.y) * (v01.y + sqrtf(v01.y * v01.y + 2.0f * (-gravity.y) * p0.y));
	Parabola(drawList, p0, v01, R2, true, axes, colour * 0.5f);

	// reset maximum point on normal curve
	max = v2(p0.x + v0.x * v0.y / (-gravity.y), p0.y + v0.y * v0.y / (2.0f * (-gravity.y)));
	if (max.x < p0.x)
		maximum.draw = false;
	else
	{
		maximum.draw = true;
		maximum.setPosGlobal(max);
	}
}

void ProjectileThroughPoint::DrawUI()
{
}
