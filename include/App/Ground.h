#pragma once
#include "Vector.h"
#include "BBox.h"

struct IntersectionResult
{
	bool intersected{ false };
	v2 position;
	v2 normal;
	float dt{};
};

struct Ground
{
	enum Type { Line, Parabola } type = Type::Line;

	void Draw(class DrawList* drawList, const bbox2& screen);

	// first is whether the intersection succeeds
	// second is the intersection point, if it does succeed
	// assumes linearity
	// dt is recalculated for the intersection and returned in the object
	IntersectionResult TestIntersect(const v2& a, const v2& b, float dt);

	bool AboveGround(const v2& p);
	bool BelowGround(const v2& p);

	v2 VerticallyNearestTo(const v2& p);

	void UI();

	float A = 1.0f;
	float B = 0.0f;
	float C = 0.0f;
};

extern inline Ground& GetGround() { static Ground g; return g; }