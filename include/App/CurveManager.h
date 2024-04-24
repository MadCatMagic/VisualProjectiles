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

	void CurveXYData(std::vector<v2> data, const v4& col, float thickness = 1.0f);
	void ParabolaData(std::vector<std::pair<v2, v2>> data, const v4& col, bool calculateDistMinMax = false, float thickness = 1.0f);
	void StaticLineXYData(std::vector<StaticLine> data, const v4& col, float thickness = 1.0f);
	
	void DrawCurves(AxisType axes, class DrawList* dl, float tCutoff);
	void ClearCurves();

private:
	struct Curve
	{
		inline virtual ~Curve() { }

		virtual void Draw(AxisType axes, DrawList* dl, float tCutoff) = 0;

		v4 col;
		float thickness{};
	};

	struct Parabola : public Curve
	{
		Parabola(std::vector<std::pair<v2, v2>> d, const v4& c, float t, bool calculateDistMinMax);

		// (x, y), (t, dist)
		std::vector<std::pair<v2, v2>> data;
		std::vector<std::pair<v2, v2>> distTurningPoints;
		void Draw(AxisType axes, DrawList* dl, float tCutoff) override;

	private:
		v2 convPos(const std::pair<v2, v2>& p, AxisType axes) const;
	};

	struct CurveXY : public Curve
	{
		CurveXY(std::vector<v2> d, const v4& c, float t);

		std::vector<v2> data;
		void Draw(AxisType axes, DrawList* dl, float tCutoff) override;
	};

	struct StaticLineXY : public Curve
	{
		StaticLineXY(std::vector<StaticLine> d, const v4& c, float t);

		std::vector<StaticLine> data;
		void Draw(AxisType axes, DrawList* dl, float tCutoff) override;
	};

	std::vector<Curve*> curves;
};

extern inline CurveManager& GetCurveManager() { static CurveManager m; return m; }