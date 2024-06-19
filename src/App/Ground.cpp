#include "App/Ground.h"

#include "Engine/DrawList.h"

void Ground::Draw(DrawList* drawList, const bbox2& screen)
{
	if (type == Type::Line)
	{
		drawList->Line(v2(screen.a.x, screen.a.x * B + C), v2(screen.b.x, screen.b.x * B + C), ImColor(1.0f, 0.0f, 1.0f, 1.0f));
	}
	else if (type == Type::Parabola)
	{
		v2 lastPoint;
		for (int i = 0; i <= 200; i++)
		{
			float x = screen.a.x + (i / 200.0f) * (screen.b.x - screen.a.x);
			v2 point = v2(x, x * (x * A + B) + C);
			if (i > 0)
				drawList->Line(lastPoint, point, ImColor(1.0f, 0.0f, 1.0f, 1.0f));
			lastPoint = point;
		}
	}
}

IntersectionResult Ground::TestIntersect(const v2& a, const v2& b, float dt)
{
	IntersectionResult r;
	v2 a_;
	v2 b_;
	// a is above the line, b is below it

	if (!BelowGround(a) && !AboveGround(b))
	{
		a_ = a;
		b_ = b;
	}
	else if (!AboveGround(a) && !BelowGround(b))
	{
		a_ = b;
		b_ = a;
	}
	else
		return r;

	if (type == Type::Line)
	{
		r.normal = v2(-B, 1.0f).normalise();
		
		if (b_.x == a_.x)
		{
			r.intersected = true;
			r.position = v2(a_.x, B * a_.x + C);
			r.dt = 0.0f;
			return r;
		}

		float k = (b_.y - a_.y) / (b_.x - a_.x);
		float x = (a_.y - k * a_.x - C) / (B - k);
		float y = B * x + C;
		r.intersected = true;
		r.position = v2(x, y);
		r.dt = dt * (a - r.position).length() / (a - b).length();
		return r;
	}
	else if (type == Type::Parabola)
	{
		if (abs(b_.x - a_.x) < 0.000001f)
		{
			r.intersected = true;
			r.position = v2(a_.x, a_.x * (a_.x * A + B) + C);
			r.dt = 0.0f;
			r.normal = v2(-(2.0f * A * a_.x + B), 1.0f).normalise();
			return r;
		}

		// equating ys
		float k = (b_.y - a_.y) / (b_.x - a_.x);
		float p = a_.y - k * a_.x;

		float x1 = (-B + k + sqrtf((B - k) * (B - k) - 4.0f * A * (C - p))) / (2.0f * A);
		float x2 = (-B + k - sqrtf((B - k) * (B - k) - 4.0f * A * (C - p))) / (2.0f * A);

		//assert(isnan(x1) || isnan(x2));

		float x = (a_.x <= x1 && x1 <= b_.x) ? x1 : x2;
		float y = x * (x * A + B) + C;
		r.intersected = true;
		r.position = v2(x, y);
		r.dt = dt * (a - r.position).length() / (a - b).length();
		r.normal = v2(-(2.0f * A * x + B), 1.0f).normalise();
		return r;
	}
	return r;
}

bool Ground::AboveGround(const v2& p)
{
	if (type == Type::Line)
		return p.y > p.x * B + C;

	else if (type == Type::Parabola)
		return p.y > p.x * (p.x * A + B) + C;

	return true;
}

bool Ground::BelowGround(const v2& p)
{
	if (type == Type::Line)
		return p.y < p.x * B + C;
	
	else if (type == Type::Parabola)
		return p.y < p.x * (p.x * A + B) + C;

	return false;
}

v2 Ground::VerticallyNearestTo(const v2& p)
{
	if (type == Type::Line)
		return v2(p.x, p.x * B + C);

	else if (type == Type::Parabola)
		return v2(p.x, p.x * (p.x * A + B) + C);
	
	return v2();
}

void Ground::UI()
{
	const char* items[] {
		"Line", "Parabola"
	};
	ImGui::Combo("ground", (int*)(& type), items, 2);
	ImGui::PushItemWidth(60.0f);
	if (type == Type::Line)
	{
		ImGui::DragFloat("m", &B, 0.1f); ImGui::SameLine();
		ImGui::DragFloat("c", &C, 0.1f);
	}
	else if (type == Type::Parabola)
	{
		ImGui::DragFloat("a", &A, 0.001f); ImGui::SameLine();
		ImGui::DragFloat("b", &B, 0.1f); ImGui::SameLine();
		ImGui::DragFloat("c", &C, 0.1f);
	}
	ImGui::PopItemWidth();
}

void Ground::LoadState(JSONType& state)
{
	type = (Type)state.obj["type"].i;
	A = (float)state.obj["A"].f;
	B = (float)state.obj["B"].f;
	C = (float)state.obj["C"].f;
}

JSONType Ground::SaveState()
{
	std::unordered_map<std::string, JSONType> map = {
		{ "type", (long)type },
		{ "A", A },
		{ "B", B },
		{ "C", C }
	};

	return { map };
}
