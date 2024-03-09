#include "App/Simulation.h"
#include "Engine/DrawList.h"

#include "imgui.h"

#include "Engine/Console.h"

void Simulation::Draw(DrawList* drawList)
{
	v2 prevPos = startPos;
	v2 vel = startVel;
	float dt = 0.05f;
	
	bool aboveGround = prevPos.x > 0.0f;
	
	for (int i = 0; i < 1000 && !(aboveGround && prevPos.x <= 0.0f); i++)
	{
		if (!aboveGround && prevPos.x > 0.0f)
			aboveGround = true;
		
		v2 newPos = prevPos + vel * dt;
		vel = vel + v2(0.0f, -9.81f) * dt;

		drawList->Line(prevPos, newPos, ImColor(colour.x, colour.y, colour.z));
		prevPos = newPos;
	}
	
}

void Simulation::DrawUI()
{
	static char buf[64] = "";
	strcpy_s(buf, name.c_str());
	
	if (ImGui::InputText("name", buf, 64))
		name = std::string(buf);

	ImGui::ColorEdit3("colour", &colour.x);

	ImGui::InputFloat2("p0", &startPos.x);
	ImGui::InputFloat2("v0", &startVel.x);
}
