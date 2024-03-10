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

void TaskOneProjectile::Draw(DrawList* drawList)
{
	v2 prevPos = startPos.position;
	v2 vel = startVel.position;
	float dt = 0.01f;
	
	bool aboveGround = prevPos.y > 0.0f;
	
	for (int i = 0; i < 1000 && !(aboveGround && prevPos.y <= 0.0f); i++)
	{
		if (!aboveGround && prevPos.y > 0.0f)
			aboveGround = true;
		
		v2 newPos = prevPos + vel * dt;
		vel = vel + gravity * dt;

		drawList->Line(prevPos, newPos, ImColor(colour.x, colour.y, colour.z));
		prevPos = newPos;
	}

	drawList->Arrow(startPos.position, startVel.getPosGlobal(), ImColor(colour.x, colour.y, colour.z));
	
}

void TaskOneProjectile::DrawUI()
{
	ImGui::InputFloat2("p0", &startPos.position.x);
	ImGui::InputFloat2("v0", &startVel.position.x);
}
