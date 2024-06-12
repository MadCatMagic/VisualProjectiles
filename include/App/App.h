#pragma once
#include "Canvas.h"

const int FRAME_TIME_MOVING_WINDOW_SIZE = 120;
const int FRAME_TIME_AVERAGE_LENGTH = 10;

// to add:
// | allow you to add nodes using a right-click on the canvas, which would place the position at the cursor aswell
//		would need extra code to account for ProjectileThroughPoint
// - larger feature: allow CurveManager to add certain 'points of interest' instead of having to use control nodes, so that
//		you could select the point in the canvas and get it to display its position - like the maxima or minima of the dist/t graph
// - a window to enable and disable different elements of a canvas (perhaps per canvas) so it does not render e.g. control nodes, interest points,
//		or whatever
// - allow the polar form of ControlVector to display the angle next to the little arc, might need to make the arc fully ccw again. connect with ^
// | lock framerate to 60fps, add way to measure frame time

class App
{
public:

	void Initialize();

	void Update();
	void UI(struct ImGuiIO* io, double averageFrameTime, double lastFrameTime);

	void Release();

	void AddSim(const std::string& name, const v2& position);

private:
	static App* activeApp;
	std::string currentFilepath = "";

	DrawStyle drawStyle;

	void DebugWindow(ImGuiIO* io, double lastFrameTime, double averageFrameTime);
	bool showDebug = false;

	float frameTimeWindow[FRAME_TIME_MOVING_WINDOW_SIZE]{ };
	float averageTimeWindow[FRAME_TIME_MOVING_WINDOW_SIZE]{ };
	int frameTimeI = 0;

	bool doCutoff = false;
	float tCutoff = 10.0f;
	float timePassed = 0.0f;
	bool playingTime = false;

	bool useRadians = true;

	void AddCanvas();

	std::vector<Canvas*> canvases;

	std::vector<class Simulation*> sims;
	std::vector<bool> simTabOpen;

	void SaveLoadWindows(bool beginSaveAs, bool beginLoad);

	void LoadState(const std::string& filename);
	void SaveState(const std::string& filename);
	
	void LoadStyle();

	static void DebugConsoleCommand(std::vector<std::string> args);
};