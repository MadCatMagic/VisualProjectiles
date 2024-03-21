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
	void AddCanvas();

	std::vector<Canvas*> canvases;

	std::vector<class Simulation*> sims;
	std::vector<bool> simTabOpen;
};