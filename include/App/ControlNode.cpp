#include "ControlNode.h"
#include "Engine/DrawList.h"

ControlNode::ControlNode()
{
	aliveNodes.push_back(this);
}

ControlNode::~ControlNode()
{
	// should never crash... hopefully
	aliveNodes.erase(std::find(aliveNodes.begin(), aliveNodes.end(), this));
}

void ControlNode::Draw(DrawList* drawList, float scale)
{
	ImColor col = ImColor(colour.x, colour.y, colour.z, colour.w);
	if (style == Style::Cross)
	{
		drawList->Line(position - v2(1.0f, 0.0f) * scale, position + v2(1.0f, 0.0f) * scale, col);
		drawList->Line(position - v2(0.0f, 1.0f) * scale, position + v2(0.0f, 1.0f) * scale, col);
	}
	else if (style == Style::Dot)
	{
		drawList->CircleFilled(position, 10.0f, col);
	}
	else if (style == Style::Circle)
	{
		drawList->Circle(position, 10.0f, col);
	}
}

std::vector<ControlNode*> ControlNode::aliveNodes = std::vector<ControlNode*>();
