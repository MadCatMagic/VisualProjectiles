#pragma once
#include "Vector.h"

#include <vector>

enum AxisType;

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
		std::pair<v2, v2> xytd;

		inline SignificantPoint() {};
		inline SignificantPoint(Type type, const v2& xy, const v2& td) : type(type), xytd(std::make_pair(xy, td)) {};
	};

	void CurveXYData(std::vector<v2> data, const v4& col, float thickness = 1.0f);
	void ParabolaData(std::vector<std::pair<v2, v2>> data, std::vector<SignificantPoint> sigPoints, const v4& col, float thickness = 1.0f);
	void StaticLineXYData(std::vector<StaticLine> data, const v4& col, float thickness = 1.0f);
	
	void DrawCurves(AxisType axes, class DrawList* dl, float tCutoff);
	void ClearCurves();

	const static uint16_t drawFlags_none = 0;
	const static uint16_t drawFlags_all = 0xFFFF;
	const static uint16_t drawFlags_maxima = 1;
	const static uint16_t drawFlags_xIntersect = 2;
	const static uint16_t drawFlags_yIntersect = 4;
	const static uint16_t drawFlags_distTurningPoints = 8;

	uint16_t drawFlags = drawFlags_all;

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

		Parabola(std::vector<std::pair<v2, v2>> d, std::vector<SignificantPoint> sigPoints, const v4& c, float t);

		// (x, y), (t, dist)
		std::vector<std::pair<v2, v2>> data;
		std::vector<SignificantPoint> significantPoints;

		void Draw(AxisType axes, DrawList* dl, float tCutoff, uint16_t flags) override;

	private:
		v2 convPos(const std::pair<v2, v2>& p, AxisType axes) const;
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