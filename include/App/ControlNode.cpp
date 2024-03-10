#include "ControlNode.h"
#include "Engine/DrawList.h"


ControlNode::ControlNode()
{
	aliveNodes.push_back(this);
}

ControlNode::ControlNode(const v2& pos, ControlNode* root)
	: position(pos), root(root)
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
	v2 pos = getPosGlobal();
	if (style == Style::Cross)
	{
		drawList->Line(pos - v2(1.0f, 0.0f) * scale, pos + v2(1.0f, 0.0f) * scale, col);
		drawList->Line(pos - v2(0.0f, 1.0f) * scale, pos + v2(0.0f, 1.0f) * scale, col);
	}
	else if (style == Style::Dot)
	{
		drawList->CircleFilled(pos, 10.0f, col);
	}
	else if (style == Style::Circle)
	{
		drawList->Circle(pos, 10.0f, col);
	}
}

std::vector<ControlNode*> ControlNode::aliveNodes = std::vector<ControlNode*>();
