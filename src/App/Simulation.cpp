#include "App/Simulation.h"
#include "Engine/DrawList.h"

#include "imgui.h"

#include "Engine/Console.h"

v2 Simulation::gravity = v2(0.0f, -9.81f);

TaskOneProjectile::TaskOneProjectile()
{
	startPos.style = ControlNode::Style::Circle;
	startPos.colour = v4(1.0f, 0.6f, 0.2f, 0.9f);
}

void TaskOneProjectile::Draw(DrawList* drawList, AxisType axes)
{
	v2 prevPos = startPos.getPosGlobal();
	v2 vel = startVel.getPosLocal();
	float dt = 0.01f;
	float t = t0;
	
	bool aboveGround = prevPos.y > 0.0f;
	
	for (int i = 0; i < 1000 && !(aboveGround && prevPos.y <= 0.0f); i++)
	{
		if (!aboveGround && prevPos.y > 0.0f)
			aboveGround = true;
		
		v2 newPos = prevPos + vel * dt;
		vel = vel + gravity * dt;
		float newt = t + dt;

		// inefficient
		if (axes == AxisType::XY)
			drawList->Line(prevPos, newPos, ImColor(colour.x, colour.y, colour.z));
		else if (axes == AxisType::XT)
			drawList->Line(v2(t, prevPos.x), v2(newt, newPos.x), ImColor(colour.x, colour.y, colour.z));
		else if (axes == AxisType::YT)
			drawList->Line(v2(t, prevPos.y), v2(newt, newPos.y), ImColor(colour.x, colour.y, colour.z));
		t = newt;
		prevPos = newPos;
	}
}

void TaskOneProjectile::DrawUI()
{
	ImGui::DragFloat("t0", &t0);
}

