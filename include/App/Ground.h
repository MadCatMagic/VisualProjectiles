#pragma once
#include "Vector.h"

struct IntersectionResult
{
	bool intersected{ false };
	v2 position;
	v2 normal;
	float dt{};
};

struct Ground
{
	enum Type { Line } type = Type::Line;

	void Draw(class DrawList* drawList, const v2& screenPos, const v2& screenSize);

	// first is whether the intersection succeeds
	// second is the intersection point, if it does succeed
	// assumes linearity
	// dt is recalculated for the intersection and returned in the object
	IntersectionResult TestIntersect(const v2& a, const v2& b, float dt);

	bool AboveGround(const v2& p);
	bool BelowGround(const v2& p);

	v2 VerticallyNearestTo(const v2& p);

	float m = 0.0f;
	float c = 0.0f;
};

extern inline Ground& GetGround() { static Ground g; return g; }