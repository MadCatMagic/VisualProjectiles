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
	bool doCutoff = false;
	float tCutoff = 10.0f;
	float timePassed = 0.0f;
	bool playingTime = false;

	bool disableControls = false;

	void AddCanvas();

	std::vector<Canvas*> canvases;

	std::vector<class Simulation*> sims;
	std::vector<bool> simTabOpen;
};