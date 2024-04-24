#include "App/CurveManager.h"

#include "App/Simulation.h"
#include "Engine/DrawList.h"

void CurveManager::CurveXYData(std::vector<v2> data, const v4& col, float thickness)
{
	curves.push_back(new CurveXY(data, col, thickness));
}

void CurveManager::ParabolaData(std::vector<std::pair<v2, v2>> data, const v4& col, bool calculateDistMinMax, float thickness)
{
	curves.push_back(new Parabola(data, col, thickness, calculateDistMinMax));
}

void CurveManager::StaticLineXYData(std::vector<StaticLine> data, const v4& col, float thickness)
{
	curves.push_back(new StaticLineXY(data, col, thickness));
}

void CurveManager::DrawCurves(AxisType axes, DrawList* dl, float tCutoff)
{
	for (Curve* curve : curves)
		curve->Draw(axes, dl, tCutoff);
}

void CurveManager::ClearCurves()
{
	for (Curve* curve : curves)
		delete curve;
	curves.clear();
}

CurveManager::Parabola::Parabola(std::vector<std::pair<v2, v2>> d, const v4& c, float t, bool calculateDistMinMax)
	: data(d)
{
	thickness = t;
	col = c;

	if (calculateDistMinMax && d.size() > 2)
	{
		float prevDir = d[1].second.y - d[0].second.y;
		bool first = true;
		for (size_t i = 0; i < d.size(); i++)
		{
			auto& pair = d[i];
			float dist = pair.second.y;
			if (first)
				first = false;
			else
			{
				float dir = dist - d[i - 1].second.y;
				if (dir > 0.0f && prevDir <= 0.0f || dir < 0.0f && prevDir >= 0.0f)
				{
					distTurningPoints.push_back(d[i - 1]);
					
					//float fraction = abs(prevDir) / abs(dir - prevDir);
					//distTurningPoints.push_back({
					//	prev.first + (pair.first - prev.first) * fraction,
					//	prev.second + (pair.second - prev.second) * fraction
					//});
				}
				prevDir = dir;
			}
		}
	}
}

void CurveManager::Parabola::Draw(AxisType axes, DrawList* dl, float tCutoff)
{
	v2 a;
	float pt = 0.0f;
	bool first = true;
	ImColor c = ImColor(col.x, col.y, col.z, col.w);
	for (auto& pair : data)
	{
		v2 b = convPos(pair, axes);

		if (first)
			first = false;
		else
		{
			float t = pair.second.x;
			if (t >= tCutoff)
			{
				b = a + (b - a) * ((tCutoff - pt) / (t - pt));
				dl->Line(a, b, c, thickness);
				return;
			}

			dl->Line(a, b, c, thickness);
		}
		a = b;
		pt = pair.second.x;
	}

	for (auto& pair : distTurningPoints)
	{
		if (pair.second.x <= tCutoff)
		{
			v2 centre = convPos(pair, axes);
			dl->Line(centre - v2(dl->scaleFactor.x * 0.5f, 0.0f), centre + v2(dl->scaleFactor.x * 0.5f, 0.0f), c, thickness);
			dl->Line(centre - v2(0.0f, dl->scaleFactor.y * 0.5f), centre + v2(0.0f, dl->scaleFactor.y * 0.5f), c, thickness);
		}
	}
}

v2 CurveManager::Parabola::convPos(const std::pair<v2, v2>& p, AxisType axes) const
{
	if (axes == AxisType::XY)
		return p.first;
	else if (axes == AxisType::XT)
		return v2(p.second.x, p.first.x);
	else if (axes == AxisType::YT)
		return v2(p.second.x, p.first.y);
	else
		return p.second;
}

CurveManager::CurveXY::CurveXY(std::vector<v2> d, const v4& c, float t)
	: data(d)
{
	thickness = t;
	col = c;
}

void CurveManager::CurveXY::Draw(AxisType axes, DrawList* dl, float tCutoff)
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

CurveManager::StaticLineXY::StaticLineXY(std::vector<StaticLine> d, const v4& c, float t)
	: data(d)
{
	thickness = t;
	col = c;
}

void CurveManager::StaticLineXY::Draw(AxisType axes, DrawList* dl, float tCutoff)
{
	if (axes != AxisType::XY)
		return;

	ImColor c = ImColor(col.x, col.y, col.z, col.w);
	for (const StaticLine& line : data)
	{
		if (line.t > tCutoff)
			return;
		dl->Line(line.a, line.b, c, thickness);
	}
}
