#pragma once
#include "Vector.h"
#include <vector>

struct ControlNode
{
	friend class Canvas;

	ControlNode();
	ControlNode(const v2& pos);
	~ControlNode();

	enum Style { Cross, Dot, Circle };

	Style style = Style::Cross;
	v4 colour = v4::one;
	std::string label = "UnnamedNode";
	
	bool positionFixed = false;

	virtual void UI();

	inline virtual void setPosGlobal(const v2& pos) { position = pos; }
	inline virtual v2 getPosGlobal() { return position; }

	inline virtual void setPosLocal(const v2& pos) { position = pos; }
	inline virtual v2 getPosLocal() { return position; }

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

	ControlNode* root = nullptr;

	virtual void setPosGlobal(const v2& pos) override;
	virtual v2 getPosGlobal() override;

	virtual void setPosLocal(const v2& pos) override;
	virtual v2 getPosLocal() override;

	virtual void UI() override;

	void setPolarness(bool isPolar);

private:
	bool usePolarDisplay = false;

	v2 polToCart(float t, float m);
	v2 cartToPol(const v2& p);
	virtual void Draw(DrawList* drawList, float scale) override;
};