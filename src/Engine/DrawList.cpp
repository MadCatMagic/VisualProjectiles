#include "Engine/DrawList.h"
#include "imgui.h"

#include "App/Simulation.h"

ImColor DrawStyle::GetCol(DrawColour colour)
{
	return colours[(int)colour].col;
}

#pragma region colours
void DrawStyle::InitColours()
{
	for (int i = 0; i < NUM_DRAW_COLOURS; i++)
	{
		ColourData c;
		switch ((DrawColour)i)
		{
		case DrawColour::Text:
			c.name = "Text";
			c.col = ImGui::GetStyleColorVec4(ImGuiCol_Text);
			break;

		case DrawColour::TextFaded:
			c.name = "TextFaded";
			c.col = ImColor(1.0f, 1.0f, 1.0f, 0.4f);
			break;

		case DrawColour::Canvas_BG:
			c.name = "Canvas_BG";
			c.col = IM_COL32(50, 50, 50, 255);
			break;

		case DrawColour::Canvas_Edge:
			c.name = "Canvas_Edge";
			c.col = IM_COL32(70, 70, 70, 255);
			break;

		case DrawColour::Canvas_GridLinesHeavy:
			c.name = "Canvas_GridLinesHeavy";
			c.col = IM_COL32(200, 200, 200, 40);
			break;

		case DrawColour::Canvas_GridLinesLight:
			c.name = "Canvas_GridLinesLight";
			c.col = IM_COL32(200, 200, 200, 20);
			break;

		case DrawColour::Canvas_Axes:
			c.name = "Canvas_Axes";
			c.col = IM_COL32(200, 200, 200, 100);
			break;
		}
		colours.push_back(c);
	}
}
#pragma endregion colours

ImVec2 DrawList::convPos(const v2& p)
{
	if (convertPosition)
		return positionCallback(p.scale(mathsWorld ? v2(10.0f, -10.0f) : v2::one)).ImGui();
	return p.ImGui();
}

void DrawList::Rect(const v2& tl, const v2& br, DrawColour col, float rounding, float thickness)
{
	Rect(tl, br, style->GetCol(col), rounding, thickness);
}

void DrawList::Rect(const v2& tl, const v2& br, const ImColor& col, float rounding, float thickness)
{
	dl->AddRect(
		convPos(tl),
		convPos(br),
		col,
		rounding,
		0,
		thickness
	);
}

void DrawList::RectFilled(const v2& tl, const v2& br, DrawColour col, float rounding, ImDrawFlags flags)
{
	RectFilled(tl, br, style->GetCol(col), rounding, flags);
}

void DrawList::RectFilled(const v2& tl, const v2& br, const ImColor& col, float rounding, ImDrawFlags flags)
{
	dl->AddRectFilled(
		convPos(tl),
		convPos(br),
		col,
		rounding,
		flags
	);
}

void DrawList::Triangle(const v2& a, const v2& b, const v2& c, DrawColour col, float thickness)
{
	Triangle(a, b, c, style->GetCol(col), thickness);
}

void DrawList::Triangle(const v2& a, const v2& b, const v2& c, const ImColor& col, float thickness)
{
	dl->AddTriangle(
		convPos(a),
		convPos(b),
		convPos(c),
		col,
		thickness
	);
}

void DrawList::TriangleFilled(const v2& a, const v2& b, const v2& c, DrawColour col)
{
	TriangleFilled(a, b, c, style->GetCol(col));
}

void DrawList::TriangleFilled(const v2& a, const v2& b, const v2& c, const ImColor& col)
{
	dl->AddTriangleFilled(
		convPos(a),
		convPos(b),
		convPos(c),
		col
	);
}

void DrawList::Circle(const v2& c, float r, DrawColour col, float thickness)
{
	Circle(c, r, style->GetCol(col), thickness);
}

void DrawList::Circle(const v2& c, float r, const ImColor& col, float thickness)
{
	dl->AddCircle(
		convPos(c),
		r,
		col,
		0,
		thickness
	);
}

void DrawList::CircleFilled(const v2& c, float r, DrawColour col)
{
	CircleFilled(c, r, style->GetCol(col));
}

void DrawList::CircleFilled(const v2& c, float r, const ImColor& col)
{
	dl->AddCircleFilled(
		convPos(c),
		r,
		col
	);
}

void DrawList::Text(const v2& p, DrawColour col, const char* text, const char* textEnd)
{
	dl->AddText(
		convPos(p),
		style->GetCol(col),
		text,
		textEnd
	);
}

void DrawList::Line(const v2& a, const v2& b, DrawColour col, float thickness)
{
	Line(a, b, style->GetCol(col), thickness);
}

void DrawList::Line(const v2& a, const v2& b, const ImColor& col, float thickness)
{
	dl->AddLine(
		convPos(a),
		convPos(b),
		col,
		thickness
	);
}

void DrawList::Arrow(const v2& a, const v2& b, const ImColor& col, float thickness)
{
	dl->AddLine(
		convPos(a),
		convPos(b),
		col,
		thickness
	);
	v2 ac = convPos(a);
	v2 bc = convPos(b);
	v2 dir = (bc - ac).normalise() * 0.5f;
	v2 rightAngle = v2(-dir.y, dir.x);
	bool convp = convertPosition;
	convertPosition = false;
	dl->AddTriangleFilled(
		convPos(bc),
		convPos(bc - (dir * 20.0f - rightAngle * 10.0f)),
		convPos(bc - (dir * 20.0f + rightAngle * 10.0f)),
		col
	);
	convertPosition = convp;
}

void DrawList::BezierCubic(const v2& a, const v2& b, const v2& c, const v2& d, DrawColour col, float thickness)
{
	dl->AddBezierCubic(
		convPos(a),
		convPos(b),
		convPos(c),
		convPos(d),
		style->GetCol(col),
		thickness
	);
}
