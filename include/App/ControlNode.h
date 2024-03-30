#pragma once
#include "Vector.h"
#include <vector>

struct ControlNode
{
	friend class App;
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

	virtual void UI(int seed, bool disable = false);

	inline virtual void setPosGlobal(const v2& pos) { position = pos; }
	inline virtual v2 getPosGlobal() const { return position; }

	inline virtual void setPosLocal(const v2& pos) { position = pos; }
	inline virtual v2 getPosLocal() const { return position; }

	bool changedThisFrame = false;

protected:
	v2 position;

	static std::vector<ControlNode*> aliveNodes;

	virtual void Draw(class DrawList* drawList, const v2& scale);
};

struct ControlVector : public ControlNode
{
	ControlVector();
	ControlVector(const v2& pos, ControlNode* root = nullptr);
	ControlVector(float theta, float magnitude, ControlNode* root = nullptr);
	virtual ~ControlVector();
	
	ControlNode* root = nullptr;

	virtual void setPosGlobal(const v2& pos) override;
	virtual v2 getPosGlobal() const override;

	virtual void setPosLocal(const v2& pos) override;
	virtual v2 getPosLocal() const override;

	v2 getPolar() const;

	virtual void UI(int seed, bool disable = false) override;

	void setPolarness(bool isPolar);

	static void setRadOrDeg(bool isRad);

	bool lockMagnitude = false;

private:
	static bool useRadians;

	bool usePolarDisplay = false;

	v2 polToCart(float t, float m) const;
	v2 cartToPol(const v2& p) const;

	virtual void Draw(DrawList* drawList, const v2& scale) override;
};