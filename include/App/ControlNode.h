#pragma once
#include "Vector.h"
#include <vector>

struct ControlNode
{
	friend class Canvas;

	ControlNode();
	ControlNode(const v2& pos);
	virtual ~ControlNode();

	enum Style { Cross, CrossDiagonal, Dot, Circle, Circross };

	Style style = Style::Cross;
	v4 colour = v4::one;
	float drawScale = 1.0f;
	std::string label = "UnnamedNode";
	
	bool positionFixed = false;
	bool draw = true;

	virtual void UI(int seed);

	inline virtual void setPosGlobal(const v2& pos) { position = pos; }
	inline virtual v2 getPosGlobal() { return position; }

	inline virtual void setPosLocal(const v2& pos) { position = pos; }
	inline virtual v2 getPosLocal() { return position; }

	bool changedThisFrame = false;

protected:
	v2 position;

	static std::vector<ControlNode*> aliveNodes;

	virtual void Draw(class DrawList* drawList, float scale);
};

struct ControlVector : public ControlNode
{
	ControlVector();
	ControlVector(const v2& pos, ControlNode* root = nullptr);
	ControlVector(float theta, float magnitude, ControlNode* root = nullptr);
	virtual ~ControlVector();
	
	ControlNode* root = nullptr;

	virtual void setPosGlobal(const v2& pos) override;
	virtual v2 getPosGlobal() override;

	virtual void setPosLocal(const v2& pos) override;
	virtual v2 getPosLocal() override;

	virtual void UI(int seed) override;

	void setPolarness(bool isPolar);

	static void setRadOrDeg(bool isRad);

	bool lockMagnitude = false;

private:
	static bool useRadians;

	bool usePolarDisplay = false;

	v2 polToCart(float t, float m);
	v2 cartToPol(const v2& p);

	virtual void Draw(DrawList* drawList, float scale) override;
};