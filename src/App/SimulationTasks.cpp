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
	if (!enabled)
		return;

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
		return;
		maximum.draw = false;
		intersectXAxis.draw = false;
	}

	// (x/g)(y+sqrt(y^2+2gh))
	float R = v0.x / (-gravity.y) * (v0.y + sqrtf(v0.y * v0.y + 2.0f * (-gravity.y) * p0.y));
	v2 pp = p0;
	float pt = 0.0f;
	int i = 0;
	for (float x = p0.x; i <= 200; x += 0.005f * R)
	{
		float tt = v0.y / v0.x;
		float rx = x - p0.x;
		float y = p0.y + rx * tt + gravity.y / (2.0f * v0.length2()) * (1.0f + tt * tt) * rx * rx;
		v2 np = v2(x, y);
		float nt = rx / p0.x;

		// plot
		drawList->Line(splitAxes(pp, pt, axes), splitAxes(np, nt, axes), ImColor(colour.x, colour.y, colour.z));

		pp = np;
		pt = nt;
		i++;
	}

	maximum.draw = true;
	intersectXAxis.draw = true;
	maximum.setPosGlobal(v2(p0.x - v0.x * v0.y / gravity.y, p0.y - v0.y * v0.y / (2.0f * gravity.y)));
	intersectXAxis.setPosGlobal(pp);
}

void TaskTwoProjectile::DrawUI()
{
}
