#include "App/Ground.h"

#include "Engine/DrawList.h"

void Ground::Draw(DrawList* drawList, const v2& screenPos, const v2& screenSize)
{
	drawList->Line(v2(-100, -100 * m + c), v2(100, 100 * m + c), ImColor(1.0f, 0.0f, 1.0f, 1.0f));
}

std::pair<bool, v2> Ground::TestIntersect(const v2& a, const v2& b)
{
	if (type == Type::Line)
	{
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
			return std::make_pair(false, v2());

		if (b_.x == a_.x)
			return std::make_pair(true, v2(a_.x, m * a_.x + c));

		float k = (b_.y - a_.y) / (b_.x - a_.x);
		float x = (a_.y - k * a_.x - c) / (m - k);
		float y = m * x + c;
		return std::make_pair(true, v2(x, y));
	}
	return std::make_pair(false, v2());
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
