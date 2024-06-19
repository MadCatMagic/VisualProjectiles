#pragma once
#include "Vector.h"

#include <vector>

enum AxisType;

struct p6
{
	inline p6() { }
	inline p6(const v2& _xy, const v2& _td, const v2& _vv) : xy(_xy), td(_td), vv(_vv) { }
	v2 xy;
	v2 td;
	v2 vv;

	p6 lerp(const p6& other, float fraction);
};

class CurveManager
{
public:

	struct StaticLine
	{
		v2 a;
		v2 b;
		float t = 0.0f;
	};

	struct SignificantPoint
	{
		enum Type { Maximum, XIntersect, YIntersect, DistTurningPoint } type = Type::Maximum;
		p6 point;

		inline SignificantPoint() {};
		inline SignificantPoint(Type type, const p6& p) : type(type), point(p) { }
		inline SignificantPoint(Type type, const v2& xy, const v2& td, const v2& vv) : type(type), point(p6(xy, td, vv)) { }
	};

	void CurveXYData(std::vector<v2> data, const v4& col, float thickness = 1.0f);
	void ParabolaData(std::vector<p6> data, std::vector<SignificantPoint> sigPoints, const v4& col, bool detectSigPoints = false, float thickness = 1.0f);
	void StaticLineXYData(std::vector<StaticLine> data, const v4& col, float thickness = 1.0f);
	
	void DrawCurves(AxisType axes, class DrawList* dl, float tCutoff);
	void ClearCurves();

	const static uint16_t drawFlags_none = 0;
	const static uint16_t drawFlags_all = 0xFFFF;
	const static uint16_t drawFlags_maxima = 1;
	const static uint16_t drawFlags_xIntersect = 2;
	const static uint16_t drawFlags_yIntersect = 4;
	const static uint16_t drawFlags_distTurningPoints = 8;

	uint16_t drawFlags = drawFlags_maxima | drawFlags_yIntersect;

private:
	struct Curve
	{
		inline virtual ~Curve() { }

		virtual void Draw(AxisType axes, DrawList* dl, float tCutoff, uint16_t flags) = 0;

		v4 col;
		float thickness{};
	};

	struct Parabola : public Curve
	{

		Parabola(std::vector<p6> d, std::vector<SignificantPoint> sigPoints, const v4& c, bool detectSigPoints, float t);

		// (x, y), (t, dist)
		std::vector<p6> data;
		std::vector<SignificantPoint> significantPoints;

		void Draw(AxisType axes, DrawList* dl, float tCutoff, uint16_t flags) override;

	private:
		v2 convPos(const p6& p, AxisType axes) const;
	};

	struct CurveXY : public Curve
	{
		CurveXY(std::vector<v2> d, const v4& c, float t);

		std::vector<v2> data;
		void Draw(AxisType axes, DrawList* dl, float tCutoff, uint16_t flags) override;
	};

	struct StaticLineXY : public Curve
	{
		StaticLineXY(std::vector<StaticLine> d, const v4& c, float t);

		std::vector<StaticLine> data;
		void Draw(AxisType axes, DrawList* dl, float tCutoff, uint16_t flags) override;
	};

	std::vector<Curve*> curves;
};

extern inline CurveManager& GetCurveManager() { static CurveManager m; return m; }