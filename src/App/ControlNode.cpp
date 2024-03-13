#include "App/ControlNode.h"
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

void ControlNode::UI(int seed)
{
	ImGui::PushID(seed);
	ImGui::InputFloat2(label.c_str(), &position.x);
	ImGui::PopID();
}

void ControlNode::Draw(DrawList* drawList, float scale)
{
	ImColor col = ImColor(colour.x, colour.y, colour.z, colour.w);
	v2 pos = getPosGlobal();
	if (style == Style::Cross)
	{
		drawList->Line(pos - v2(1.0f, 0.0f) * scale * drawScale, pos + v2(1.0f, 0.0f) * scale * drawScale, col);
		drawList->Line(pos - v2(0.0f, 1.0f) * scale * drawScale, pos + v2(0.0f, 1.0f) * scale * drawScale, col);
	}
	else if (style == Style::CrossDiagonal)
	{
		drawList->Line(pos - v2(0.707f, 0.707f) * scale * drawScale, pos + v2(0.707f, 0.707f) * scale * drawScale, col);
		drawList->Line(pos - v2(-0.707f, 0.707f) * scale * drawScale, pos + v2(-0.707f, 0.707f) * scale * drawScale, col);
	}
	else if (style == Style::Dot)
	{
		drawList->CircleFilled(pos, 10.0f * drawScale, col);
	}
	else if (style == Style::Circle)
	{
		drawList->Circle(pos, 10.0f * drawScale, col);
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

void ControlVector::UI(int seed)
{
	static bool polCheck = false;
	ImGui::PushID(seed);
	ImGui::PushItemWidth(60.0f);
	if (useRadians)
		ImGui::InputFloat("rad", &position.x);
	else
		ImGui::InputFloat("deg", &position.x);
	ImGui::SameLine();
	ImGui::InputFloat("mag", &position.y);
	ImGui::PopItemWidth();

	if (ImGui::Checkbox((label + " is polar?").c_str(), &polCheck))
		setPolarness(polCheck);
	ImGui::PopID();
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

bool ControlVector::useRadians = true;

void ControlVector::setRadOrDeg(bool isRad)
{
	if (useRadians == isRad)
		return;

	// from degrees to radians
	if (isRad && !useRadians)
		for (ControlNode* node : aliveNodes)
		{
			ControlVector* vec = dynamic_cast<ControlVector*>(node);
			if (vec != nullptr && vec->usePolarDisplay)
				vec->position.x *= PI / 180.0f;
		}
	// from radians to degrees
	else
		for (ControlNode* node : aliveNodes)
		{
			ControlVector* vec = dynamic_cast<ControlVector*>(node);
			if (vec != nullptr && vec->usePolarDisplay)
				vec->position.x *= 180.0f / PI;
		}

	useRadians = isRad;
}

v2 ControlVector::polToCart(float t, float m)
{
	if (useRadians)
		return v2(cosf(t) * m, sinf(t) * m);
	float theta = t * (PI / 180.0f);
	return v2(cosf(theta) * m, sinf(theta) * m);
}

v2 ControlVector::cartToPol(const v2& p)
{
	if (abs(p.x) < 0.0000001f)
	{
		v2 k = v2(PI * (0.5f - 1.0f * (p.y < 0.0f)), sqrtf(p.x * p.x + p.y * p.y));
		k.x = k.x < 0.0f ? k.x + PI * 2 : k.x;
		if (useRadians)
			return k;
		return k.scale(v2(180.0f / PI, 1.0f));
	}

	float f = atanf(p.y / p.x) + PI * (1.0f - (p.x > 0.0f));
	v2 k = v2(f < 0.0f ? f + PI * 2 : f, sqrtf(p.x * p.x + p.y * p.y));
	if (useRadians)
		return k;
	return k.scale(v2(180.0f / PI, 1.0f));
}

void ControlVector::Draw(DrawList* drawList, float scale)
{
	if (root != nullptr)
	{
		v2 gp = root->getPosGlobal();
		drawList->Arrow(gp, getPosGlobal(), ImColor(1.0f, 0.0f, 1.0f, 0.8f));

		if (usePolarDisplay)
		{
			float pi = 0.0f;
			float polarSize = 2.0f;
			float theta = useRadians ? position.x : position.x * (PI / 180.0f);
			for (float i = std::min(0.1f, theta);; i = std::min(i + 0.1f, theta))
			{
				drawList->Line(gp + v2(cosf(pi), sinf(pi)) * polarSize, gp + v2(cosf(i), sinf(i)) * polarSize, DrawColour::Canvas_GridLinesHeavy);
				pi = i;
				if (i == theta)
					break;
			}
			drawList->Line(gp, gp + v2(polarSize, 0.0f), DrawColour::Canvas_GridLinesHeavy);
		}
	}
	else
	{
		drawList->Arrow(v2(), getPosGlobal(), ImColor(1.0f, 0.0f, 1.0f, 0.8f));
	}

	ControlNode::Draw(drawList, scale);
}
