#include "App/Ground.h"

#include "Engine/DrawList.h"

void Ground::Draw(DrawList* drawList, const v2& screenPos, const v2& screenSize)
{
	drawList->Line(v2(-100, -100 * m + c), v2(100, 100 * m + c), ImColor(1.0f, 0.0f, 1.0f, 1.0f));
}

IntersectionResult Ground::TestIntersect(const v2& a, const v2& b, float dt)
{
	IntersectionResult r;
	if (type == Type::Line)
	{
		r.normal = v2(-m, 1.0f).normalise();
		// solve y > mx + c
		// a is above the line, b is below it
		v2 a_;
		v2 b_;
		if ((a.y >= m * a.x + c) && (b.y <= m * b.x + c))
		{
			a_ = a;
			b_ = b;
		}
		else if ((a.y <= m * a.x + c) && (b.y >= m * b.x + c))
		{
			a_ = b;
			b_ = a;
		}
		else
			return r;

		if (b_.x == a_.x)
		{
			r.intersected = true;
			r.position = v2(a_.x, m * a_.x + c);
			r.dt = 0.0f;
			return r;
		}

		float k = (b_.y - a_.y) / (b_.x - a_.x);
		float x = (a_.y - k * a_.x - c) / (m - k);
		float y = m * x + c;
		r.intersected = true;
		r.position = v2(x, y);
		r.dt = dt * (a - r.position).length() / (a - b).length();
		return r;
	}
	return r;
}

bool Ground::AboveGround(const v2& p)
{
	if (type == Type::Line)
	{
		return p.y > p.x * m + c;
	}
	return true;
}

bool Ground::BelowGround(const v2& p)
{
	if (type == Type::Line)
	{
		return p.y < p.x * m + c;
	}
	return true;
}

v2 Ground::VerticallyNearestTo(const v2& p)
{
	if (type == Type::Line)
	{
		return v2(p.x, p.x * m + c);
	}
	return v2();
}
