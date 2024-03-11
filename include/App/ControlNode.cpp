#include "ControlNode.h"
#include "Engine/DrawList.h"

#include "imgui.h"

ControlNode::ControlNode()
{
	aliveNodes.push_back(this);
}

ControlNode::ControlNode(const v2& pos)
	: position(pos)
{
	aliveNodes.push_back(this);
}

ControlNode::~ControlNode()
{
	// should never crash... hopefully
	aliveNodes.erase(std::find(aliveNodes.begin(), aliveNodes.end(), this));
}

void ControlNode::UI()
{
	ImGui::InputFloat2(label.c_str(), &position.x);
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

ControlVector::ControlVector()
	: ControlNode()
{ }

ControlVector::ControlVector(const v2& pos, ControlNode* root)
	: ControlNode(pos), root(root)
{ }

ControlVector::ControlVector(float theta, float magnitude, ControlNode* root)
	: ControlNode(v2(theta, magnitude)), root(root), usePolarDisplay(true)
{ }

void ControlVector::setPosGlobal(const v2& pos)
{
	if (root == nullptr)
		setPosLocal(pos);
	else
		setPosLocal(pos - root->getPosGlobal());
}

v2 ControlVector::getPosGlobal()
{
	if (root == nullptr)
		return getPosLocal();
	else
		return getPosLocal() + root->getPosGlobal();
}

void ControlVector::setPosLocal(const v2& pos)
{
	if (usePolarDisplay)
		position = cartToPol(pos);
	else
		position = pos;
}

v2 ControlVector::getPosLocal()
{
	if (usePolarDisplay)
		return polToCart(position.x, position.y);
	else
		return position;
}

void ControlVector::UI()
{
	static bool polCheck = false;
	ImGui::InputFloat2(label.c_str(), &position.x);
	if (ImGui::Checkbox((label + "is polar?").c_str(), &polCheck))
		setPolarness(polCheck);
}

void ControlVector::setPolarness(bool isPolar)
{
	if (usePolarDisplay == isPolar)
		return;

	if (!usePolarDisplay && isPolar)
		position = cartToPol(position);
	else
		position = polToCart(position.x, position.y);
	usePolarDisplay = isPolar;
}

v2 ControlVector::polToCart(float t, float m)
{
	return v2(cosf(t) * m, sinf(t) * m);
}

v2 ControlVector::cartToPol(const v2& p)
{
	if (abs(p.x) < 0.0000001f)
		return v2(PI * (0.5f - 1.0f * (p.y < 0.0f)), sqrtf(p.x * p.x + p.y * p.y));

	return v2(atanf(p.y / p.x) + PI * (1.0f - (p.x > 0.0f)), sqrtf(p.x * p.x + p.y * p.y));
}

void ControlVector::Draw(DrawList* drawList, float scale)
{
	if (root != nullptr)
	{
		v2 gp = root->getPosGlobal();
		drawList->Arrow(gp, getPosGlobal(), ImColor(1.0f, 0.0f, 1.0f, 0.8f));

		if (usePolarDisplay)
		{
			v2 ucp = v2(cosf(position.x), sinf(position.y));
			float l = 4.0f / 3.0f * tanf(v2(1.0f, 0.0f).distanceTo(ucp) * 0.25f);
			v2 p0 = gp + v2(1.0f, 0.0f);
			v2 p1 = gp + ucp;
			// does not work
			drawList->BezierCubic(
				p0,
				p0 + v2(0.0f, 1.0f) * l,
				p1 + v2(ucp.y, -ucp.x) * l,
				p1,
				DrawColour::Text
			);
		}
	}
	else
	{
		drawList->Arrow(v2(), getPosGlobal(), ImColor(1.0f, 0.0f, 1.0f, 0.8f));
	}

	ControlNode::Draw(drawList, scale);
}
