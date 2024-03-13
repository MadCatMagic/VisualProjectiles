#include "App/SimulationTasks.h"
#include "App/Ground.h"

#include "Engine/DrawList.h"

#include "imgui.h"

TaskOneProjectile::TaskOneProjectile()
{
	startPos.style = ControlNode::Style::Circle;
	startPos.colour = v4(1.0f, 0.6f, 0.2f, 0.9f);
	startPos.label = "p0";
	startVel.label = "v0";
	startVel.style = ControlNode::Style::Circle;
	startVel.colour = v4(1.0f, 0.6f, 0.2f, 0.9f);

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