#include "App/CurveManager.h"

#include "App/Simulation.h"
#include "Engine/DrawList.h"

void CurveManager::CurveXYData(std::vector<v2> data, const v4& col, float thickness)
{
	curves.push_back(new CurveXY(data, col, thickness));
}

void CurveManager::ParabolaData(std::vector<std::pair<v2, v2>> data, const v4& col, float thickness)
{
	curves.push_back(new Parabola(data, col, thickness ));
}

void CurveManager::DrawCurves(AxisType axes, DrawList* dl)
{
	for (Curve* curve : curves)
		curve->Draw(axes, dl);
}

void CurveManager::ClearCurves()
{
	for (Curve* curve : curves)
		delete curve;
	curves.clear();
}

CurveManager::Parabola::Parabola(std::vector<std::pair<v2, v2>> d, const v4& c, float t)
	: data(d)
{
	thickness = t;
	col = c;
}

void CurveManager::Parabola::Draw(AxisType axes, DrawList* dl)
{
	v2 a;
	bool first = true;
	ImColor c = ImColor(col.x, col.y, col.z, col.w);
	for (auto& pair : data)
	{
		v2 b;
		if (axes == AxisType::XY)
			b = pair.first;
		else if (axes == AxisType::XT)
			b = v2(pair.second.x, pair.first.x);
		else if (axes == AxisType::YT)
			b = v2(pair.second.x, pair.first.y);
		else
			b = pair.second;
		if (first)
			first = false;
		else
			dl->Line(a, b, c, thickness);
		a = b;
	}
}

CurveManager::CurveXY::CurveXY(std::vector<v2> d, const v4& c, float t)
	: data(d)
{
	thickness = t;
	col = c;
}

void CurveManager::CurveXY::Draw(AxisType axes, DrawList* dl)
{
	if (axes != AxisType::XY)
		return;

	v2 a;
	bool first = true;
	ImColor c = ImColor(col.x, col.y, col.z, col.w);
	for (v2& b : data)
	{
		if (first)
			first = false;
		else
			dl->Line(a, b, c, thickness);
		a = b;
	}
}
