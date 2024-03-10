#pragma once
#include "Vector.h"
#include <vector>

struct ControlNode
{
	friend class Canvas;

	ControlNode();
	ControlNode(const v2& pos, ControlNode* root = nullptr);
	~ControlNode();

	enum Style { Cross, Dot, Circle };

	Style style = Style::Cross;
	v4 colour = v4::one;
	std::string label;
	
	v2 position;
	bool positionFixed = false;

	ControlNode* root = nullptr;
	
	inline void setPosGlobal(const v2& pos) { position = root == nullptr ? pos : pos - root->position; }
	inline v2 getPosGlobal() { return root == nullptr ? position : position + root->position; }

private:
	static std::vector<ControlNode*> aliveNodes;

	void Draw(class DrawList* drawList, float scale);
};