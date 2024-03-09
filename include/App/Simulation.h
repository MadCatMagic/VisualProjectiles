#pragma once
#include "Vector.h"

class Simulation
{
public:

	void Draw(class DrawList* drawList);
	void DrawUI();

	std::string name = "Squidward";

private:
	v3 colour = v3::one;
	v2 startPos;
	v2 startVel;
};