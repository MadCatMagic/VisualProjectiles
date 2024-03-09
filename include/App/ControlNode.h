#pragma once
#include "Vector.h"
#include <vector>

struct ControlNode
{
	friend class Canvas;

	ControlNode();
	~ControlNode();

	enum Style { Cross, Dot, Circle };

	Style style = Style::Cross;
	v4 colour = v4::one;
	std::string label;
	
	v2 position;
	bool positionFixed = false;

private:
	static std::vector<ControlNode*> aliveNodes;

	void Draw(class DrawList* drawList, float scale);
};