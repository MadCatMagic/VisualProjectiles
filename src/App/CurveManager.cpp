#include "App/CurveManager.h"

#include "App/Simulation.h"
#include "Engine/DrawList.h"

p6 p6::lerp(const p6& other, float fraction)
{
	return p6(
		xy * fraction + other.xy * (1.0f - fraction),
		td * fraction + other.td * (1.0f - fraction),
		vv * fraction + other.vv * (1.0f - fraction)
	);
}

void CurveManager::CurveXYData(std::vector<v2> data, const v4& col, float thickness)
{
	curves.push_back(new CurveXY(data, col, thickness));
}

void CurveManager::ParabolaData(std::vector<p6> data, std::vector<SignificantPoint> sigPoints, const v4& col, bool detectSigPoints, float thickness)
{
	curves.push_back(new Parabola(data, sigPoints, col, detectSigPoints, thickness));
}

void CurveManager::StaticLineXYData(std::vector<StaticLine> data, const v4& col, float thickness)
{
	curves.push_back(new StaticLineXY(data, col, thickness));
}

void CurveManager::DrawCurves(AxisType axes, DrawList* dl, float tCutoff)
{
	for (Curve* curve : curves)
		curve->Draw(axes, dl, tCutoff, drawFlags);
}

void CurveManager::ClearCurves()
{
	for (Curve* curve : curves)
		delete curve;
	curves.clear();
}

CurveManager::Parabola::Parabola(std::vector<p6> d, std::vector<SignificantPoint> sigPoints, const v4& c, bool detectSigPoints, float t)
	: data(d)
{
	thickness = t;
	col = c;

	for (auto& e : sigPoints)
		significantPoints.push_back(e);

	// nothing to detect if array too small, return
	if (d.size() <= 2) return;

	// always detect dist maxima/minima
	float prevDir = d[1].td.y - d[0].td.y;
	bool first = true;
	for (size_t i = 0; i < d.size(); i++)
	{
		auto& pair = d[i];
		float dist = pair.td.y;
		if (first)
			first = false;
		else
		{
			float dir = dist - d[i - 1].td.y;
			if (dir > 0.0f && prevDir <= 0.0f || dir < 0.0f && prevDir >= 0.0f)
			{
				SignificantPoint o;
				o.type = SignificantPoint::Type::DistTurningPoint;
				o.point = d[i - 1];
				significantPoints.push_back(o);
					
				//float fraction = abs(prevDir) / abs(dir - prevDir);
				//distTurningPoints.push_back({
				//	prev.first + (pair.first - prev.first) * fraction,
				//	prev.second + (pair.second - prev.second) * fraction
				//});
			}
			prevDir = dir;
		}
	}

	// only detect other ones if requested to
	if (!detectSigPoints)
		return;
	
	p6& prevPoint = d[0];
	bool increasing = d[0].xy.y > d[1].xy.y;
	// y-intersects
	// x-intersects
	// maximum points

	for (size_t i = 1; i < d.size(); i++)
	{
		// maximum point
		if (increasing && d[i].xy.y < prevPoint.xy.y)
		{
			increasing = false;
			significantPoints.push_back({ SignificantPoint::Type::Maximum, prevPoint });
		}
		else if (!increasing && d[i].xy.y > prevPoint.xy.y)
			increasing = true;

		// y-intersect
		if (prevPoint.xy.x <= 0.0f && d[i].xy.x > 0.0f || prevPoint.xy.x > 0.0f && d[i].xy.x <= 0.0f)
		{
			float fraction = -prevPoint.xy.x / (d[i].xy.x - prevPoint.xy.x);
			significantPoints.push_back({ 
				SignificantPoint::Type::YIntersect, 
				d[i].lerp(prevPoint, fraction)
			});
		}

		// x-intersect
		if (prevPoint.xy.y <= 0.0f && d[i].xy.y > 0.0f || prevPoint.xy.y > 0.0f && d[i].xy.y <= 0.0f)
		{
			float fraction = -prevPoint.xy.y / (d[i].xy.y - prevPoint.xy.y);
			significantPoints.push_back({
				SignificantPoint::Type::XIntersect,
				d[i].lerp(prevPoint, fraction)
			});
		}

		prevPoint = d[i];
	}
}

void CurveManager::Parabola::Draw(AxisType axes, DrawList* dl, float tCutoff, uint16_t flags)
{
	v2 a;
	float pt = 0.0f;
	bool first = true;
	ImColor c = ImColor(col.x, col.y, col.z, col.w);
	for (p6& point : data)
	{
		v2 b = convPos(point, axes);

		if (first)
			first = false;
		else
		{
			float t = point.td.x;
			if (t >= tCutoff)
			{
				b = a + (b - a) * ((tCutoff - pt) / (t - pt));
				dl->Line(a, b, c, thickness);
				return;
			}

			dl->Line(a, b, c, thickness);
		}
		a = b;
		pt = point.td.x;
	}

	for (auto& point : significantPoints)
	{
		if (
			point.type == SignificantPoint::Type::DistTurningPoint && !(flags & drawFlags_distTurningPoints) ||
			point.type == SignificantPoint::Type::Maximum && !(flags & drawFlags_maxima) ||
			point.type == SignificantPoint::Type::XIntersect && !(flags & drawFlags_xIntersect) ||
			point.type == SignificantPoint::Type::YIntersect && !(flags & drawFlags_yIntersect)
		)
			continue;

		const ImColor sigPointColours[4] = {
			ImColor(1.0f, 0.6f, 0.2f), // maximum
			ImColor(0.2f, 0.9f, 1.0f), // xinter
			ImColor(0.0f, 1.0f, 0.3f), // yinter
			ImColor(0.8f, 1.0f, 0.1f)  // disttp
		};

		if (point.point.td.x <= tCutoff)
		{
			const float ll = 0.7f;
			v2 centre = convPos(point.point, axes);
			dl->Line(
				centre - v2(dl->scaleFactor.x * ll, 0.0f),
				centre + v2(dl->scaleFactor.x * ll, 0.0f),
				sigPointColours[(int)point.type], 
				2.0f
			);
			dl->Line(
				centre - v2(0.0f, dl->scaleFactor.y * ll),
				centre + v2(0.0f, dl->scaleFactor.y * ll),
				sigPointColours[(int)point.type], 
				2.0f
			);
		}
	}
}

v2 CurveManager::Parabola::convPos(const p6& p, AxisType axes) const
{
	switch (axes)
	{
	case AxisType::XY:    return p.xy;
	case AxisType::XT:    return v2(p.td.x, p.xy.x);
	case AxisType::YT:    return v2(p.td.x, p.xy.y);
	case AxisType::DistT: return p.td;
	case AxisType::VxT:   return v2(p.td.x, p.vv.x);
	case AxisType::VyT:   return v2(p.td.x, p.vv.y);
	case AxisType::VT:    return v2(p.td.x, p.vv.length());
	}
	return v2();
}

CurveManager::CurveXY::CurveXY(std::vector<v2> d, const v4& c, float t)
	: data(d)
{
	thickness = t;
	col = c;
}

void CurveManager::CurveXY::Draw(AxisType axes, DrawList* dl, float tCutoff, uint16_t flags)
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

void CurveManager::StaticLineXY::Draw(AxisType axes, DrawList* dl, float tCutoff, uint16_t flags)
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
