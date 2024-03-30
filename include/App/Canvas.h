#pragma once
#include "Vector.h"
#include "Engine/DrawList.h"

// will act as a file which contains all of the nodes, but only as a reference grid point
// all of the transformations between local and canvas coordinates happen here
// as well as all of the maths for scaling and moving around and such
// but all node interactions happen elsewhere

// CreateWindow renders everything, including all the nodes that are passed to it.
#define NUM_SCALING_LEVELS 32
#define MIN_SCALE 0.06490547151f

enum AxisType;

class Canvas
{
public:
	inline Canvas() {}
	~Canvas();

	void InitCanvas();
	void CreateWindow(std::vector<class Simulation*>& sims, int window_N, float tCutoff, bool disableControls);

	v2 ScreenToCanvas(const v2& pos) const;
	v2 CanvasToScreen(const v2& pos) const;
	v2 CanvasToPosition(const v2& pos) const;
	v2 PositionToCanvas(const v2& pos) const;

	inline v2 GetSF() const { return scale; }
	v2 GetSFFromScalingLevel(const v2i& scaling);

	// shortcut
	inline v2 ptcts(const v2& pos) const { return CanvasToScreen(PositionToCanvas(pos)); }
	inline v2 stctp(const v2& pos) const { return CanvasToPosition(ScreenToCanvas(pos)); }

	
private:
	std::string pprint(float n, float s);

	float fv{};
	const int pixelsPerUnit = 10;

	// text stuff
	struct ImFont* textLODs[NUM_SCALING_LEVELS]{};

	v2i scalingLevel;
	v2 position = v2(-300, -300);
	v2 scale = v2::one;

	v2 canvasPixelPos;
	v2 canvasPixelSize;

	DrawList drawList;

	// all used in the CreateWindow func
	struct ControlNode* selectedControlNode = nullptr;
	bool draggingControlNode = false;
	v2 originalCNPosition;
	v2 cumulativeCNOffset;

	AxisType axisType = (AxisType)0;
	v2 previousWindowSize;
};