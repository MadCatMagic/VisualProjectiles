#include "App/Simulations/SimpleProjectile.h"

#include "App/Ground.h"
#include "Engine/DrawList.h"

SimpleProjectile::SimpleProjectile()
{
	intersectXAxis.draw = false;
	intersectXAxis.style = ControlNode::Style::CrossDiagonal;
	intersectXAxis.colour = v4(1.0f);
	intersectXAxis.positionFixed = true;
}

void SimpleProjectile::Draw(DrawList* drawList, AxisType axes)
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
	ImGui::DragFloat("t0", &t0);
}