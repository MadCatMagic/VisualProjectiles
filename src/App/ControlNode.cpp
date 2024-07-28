#include "App/ControlNode.h"
#include "Engine/DrawList.h"

#include "imgui.h"

#include "Engine/Console.h"

ControlNode::ControlNode()
{
	aliveNodes.push_back(this);
}

ControlNode::ControlNode(const v2& pos)
	: position(pos)
{
	aliveNodes.push_back(this);
}

ControlNode::ControlNode(JSONType& state)
{
	style = (Style)state.obj["style"].i;
	auto& colourVec = state.obj["colour"].arr;
	colour = v4((float)colourVec[0].f, (float)colourVec[1].f, (float)colourVec[2].f, (float)colourVec[3].f);
	drawScale = (float)state.obj["drawScale"].f;
	label = state.obj["label"].s;

	positionFixed = state.obj["positionFixed"].b;
	draw = state.obj["draw"].b;

	auto& posVec = state.obj["position"].arr;
	position = v2((float)posVec[0].f, (float)posVec[1].f);

	aliveNodes.push_back(this);
}

ControlNode::~ControlNode()
{
	// should never crash... hopefully
	aliveNodes.erase(std::find(aliveNodes.begin(), aliveNodes.end(), this));
}

void ControlNode::UI(int seed, bool disable)
{
	ImGui::BeginDisabled(disable);
	ImGui::PushID(seed);
	if (ImGui::InputFloat2(label.c_str(), &position.x))
		changedThisFrame = true;
	ImGui::PopID();
	ImGui::EndDisabled();
}

JSONType ControlNode::SaveState()
{
	std::unordered_map<std::string, JSONType> map = {
		{ "style", (long)style },
		{ "colour", colour },
		{ "drawScale", drawScale },
		{ "label", label },
		{ "positionFixed", positionFixed },
		{ "draw", draw },
		{ "position", position },
		{ "id", GetID() }
	};
	return { map };
}

// only cared about at save/load time
// so position is constant
std::string ControlNode::GetID()
{
	return std::to_string(style) + colour.str() + label + position.str();
}

ControlNode* ControlNode::NodeFromID(const std::string& id)
{
	for (ControlNode* node : aliveNodes)
		if (node->GetID() == id)
			return node;
		
	return nullptr;
}

void ControlNode::Draw(DrawList* drawList, const v2& scale)
{
	ImColor col = ImColor(colour.x, colour.y, colour.z, colour.w);
	v2 pos = getPosGlobal();
	if (style == Style::Cross)
	{
		drawList->Line(pos - v2(1.0f, 0.0f).scale(scale) * drawScale, pos + v2(1.0f, 0.0f).scale(scale) * drawScale, col);
		drawList->Line(pos - v2(0.0f, 1.0f).scale(scale) * drawScale, pos + v2(0.0f, 1.0f).scale(scale) * drawScale, col);
	}
	else if (style == Style::CrossDiagonal)
	{
		drawList->Line(pos - v2(0.707f, 0.707f).scale(scale) * drawScale, pos + v2(0.707f, 0.707f).scale(scale) * drawScale, col);
		drawList->Line(pos - v2(-0.707f, 0.707f).scale(scale) * drawScale, pos + v2(-0.707f, 0.707f).scale(scale) * drawScale, col);
	}
	else if (style == Style::Dot)
	{
		drawList->CircleFilled(pos, 10.0f * drawScale, col);
	}
	else if (style == Style::Circle)
	{
		drawList->Circle(pos, 10.0f * drawScale, col);
	}
	else if (style == Style::Circross)
	{
		drawList->Circle(pos, 10.0f * drawScale, col);
		drawList->Line(pos - v2(1.0f, 0.0f).scale(scale) * drawScale, pos + v2(1.0f, 0.0f).scale(scale) * drawScale, col);
		drawList->Line(pos - v2(0.0f, 1.0f).scale(scale) * drawScale, pos + v2(0.0f, 1.0f).scale(scale) * drawScale, col);
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

ControlVector::ControlVector(JSONType& state)
	: ControlNode(state)
{
	// TODO:: ++++
	// the root might not yet exist...
	// root = ;
	if (state.obj["root"].s != "NONE")
		toRoot.push_back(std::make_pair(GetID(), state.obj["root"].s));
	lockMagnitude = state.obj["lockMagnitude"].b;
	usePolarDisplay = state.obj["usePolarDisplay"].b;
}

ControlVector::~ControlVector()
{ }

void ControlVector::setPosGlobal(const v2& pos)
{
	if (root == nullptr)
		setPosLocal(pos);
	else
		setPosLocal(pos - root->getPosGlobal());
}

v2 ControlVector::getPosGlobal() const
{
	if (root == nullptr)
		return getPosLocal();
	else
		return getPosLocal() + root->getPosGlobal();
}

void ControlVector::setPosLocal(const v2& pos)
{
	if (usePolarDisplay)
	{
		if (lockMagnitude)
			position = v2(cartToPol(pos).x, position.y);
		else
			position = cartToPol(pos);
	}
	else
		position = pos;
}

v2 ControlVector::getPolar() const
{
	if (usePolarDisplay)
		return position.scale(useRadians ? 1.0f : v2(PI / 180.0f, 1.0f));
	return cartToPol(position);
}

v2 ControlVector::getPosLocal() const
{
	if (usePolarDisplay)
		return polToCart(position.x, position.y);
	else
		return position;
}

void ControlVector::UI(int seed, bool disable)
{
	ImGui::BeginDisabled(disable);

	// disgusting :P
	ImGui::PushID(seed);
	if (usePolarDisplay)
	{
		ImGui::PushItemWidth(60.0f);
		if (useRadians)
		{
			if (ImGui::InputFloat("rad", &position.x))
				changedThisFrame = true;
		}
		else
		{
			if (ImGui::InputFloat("deg", &position.x))
				changedThisFrame = true;
		}
		ImGui::SameLine();
		if (ImGui::InputFloat("mag", &position.y))
			changedThisFrame = true;
		ImGui::PopItemWidth();
	}
	else
		if (ImGui::InputFloat2(label.c_str(), &position.x))
			changedThisFrame = true;

	bool polCheck = usePolarDisplay;
	if (ImGui::Checkbox((label + " is polar?").c_str(), &polCheck))
		setPolarness(polCheck);
	if (usePolarDisplay)
		ImGui::Checkbox((label + " lock magnitude").c_str(), &lockMagnitude);
	ImGui::PopID();

	ImGui::EndDisabled();
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

JSONType ControlVector::SaveState()
{
	std::unordered_map<std::string, JSONType> map = {
		{ "style", (long)style },
		{ "colour", colour },
		{ "drawScale", drawScale },
		{ "label", label },
		{ "positionFixed", positionFixed },
		{ "draw", draw },
		{ "position", position },
		{ "root", root == nullptr ? "NONE" : root->GetID() },
		{ "lockMagnitude", lockMagnitude },
		{ "usePolarDisplay", usePolarDisplay }
	};
	return { map };
}

std::vector<std::pair<std::string, std::string>> ControlVector::toRoot = std::vector<std::pair<std::string, std::string>>();
void ControlVector::Root()
{
	for (auto& p : toRoot)
	{
		ControlNode* n = NodeFromID(p.first);
		if (n == nullptr)
			continue;

		((ControlVector*)n)->root = NodeFromID(p.second);
	}
	toRoot.clear();
}

v2 ControlVector::polToCart(float t, float m) const
{
	if (useRadians)
		return v2(cosf(t) * m, sinf(t) * m);
	float theta = t * (PI / 180.0f);
	return v2(cosf(theta) * m, sinf(theta) * m);
}

v2 ControlVector::cartToPol(const v2& p) const
{
	if (p.y == 0.0f && p.x == 0.0f)
		return v2(0.0f, 0.0f);

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

void ControlVector::Draw(DrawList* drawList, const v2& scale)
{
	if (root != nullptr)
	{
		v2 gp = root->getPosGlobal();
		drawList->Arrow(gp, getPosGlobal(), ImColor(1.0f, 0.0f, 1.0f, 0.8f));

		if (usePolarDisplay)
		{
			float theta = useRadians ? position.x : position.x * (PI / 180.0f);
			float pi =
				(theta <= PI * 0.5f) ? 0.0f :
				(theta >= PI * 1.5f) ? 2.0f * PI :
				PI;
			float polarSize = getPosLocal().length() * 0.15f;
			if (theta <= PI * 0.5f || PI <= theta && theta <= PI * 1.5f)
				for (float i = std::min(pi + 0.1f, theta);; i = std::min(i + 0.1f, theta))
				{
					drawList->Line(gp + v2(cosf(pi), sinf(pi)) * polarSize, gp + v2(cosf(i), sinf(i)) * polarSize, DrawColour::Canvas_GridLinesHeavy);
					pi = i;
					if (i == theta)
						break;
				}
			else
				for (float i = std::max(pi - 0.1f, theta);; i = std::max(i - 0.1f, theta))
				{
					drawList->Line(gp + v2(cosf(pi), sinf(pi)) * polarSize, gp + v2(cosf(i), sinf(i)) * polarSize, DrawColour::Canvas_GridLinesHeavy);
					pi = i;
					if (i == theta)
						break;
				}
			if (theta <= PI * 0.5f || theta >= PI * 1.5f)
				drawList->Line(gp, gp + v2(polarSize, 0.0f), DrawColour::Canvas_GridLinesHeavy);
			else
				drawList->Line(gp, gp - v2(polarSize, 0.0f), DrawColour::Canvas_GridLinesHeavy);
		}
	}
	else
	{
		drawList->Arrow(v2(), getPosGlobal(), ImColor(1.0f, 0.0f, 1.0f, 0.8f));
	}

	ControlNode::Draw(drawList, scale);
}
