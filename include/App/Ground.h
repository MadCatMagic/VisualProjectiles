#pragma once
#include "Vector.h"

struct Ground
{
	enum Type { Line } type = Type::Line;

	void Draw(class DrawList* drawList, const v2& screenPos, const v2& screenSize);

	// first is whether the intersection succeeds
	// second is the intersection point, if it does succeed
	// assumes linearity
	std::pair<bool, v2> TestIntersect(const v2& a, const v2& b);

	bool AboveGround(const v2& p);
	bool BelowGround(const v2& p);

	v2 VerticallyNearestTo(const v2& p);

	float m = 0.0f;
	float c = 0.0f;
};

extern inline Ground& GetGround() { static Ground g; return g; }