#pragma once
#include "Canvas.h"

class App
{
public:

	void Initialize();

	void Update();
	void UI(struct ImGuiIO* io);

	void Release();

private:
	Canvas c;
	std::vector<class Simulation*> sims;
	std::vector<bool> simTabOpen;
};