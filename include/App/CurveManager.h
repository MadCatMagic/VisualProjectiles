#pragma once
#include "Vector.h"

#include <vector>

enum AxisType;

class CurveManager
{
public:
	void CurveXYData(std::vector<v2> data, const v4& col, float thickness = 1.0f);
	void ParabolaData(std::vector<std::pair<v2, v2>> data, const v4& col, float thickness = 1.0f);
	void DrawCurves(AxisType axes, class DrawList* dl);

	void ClearCurves();

private:
	struct Curve
	{
		inline virtual ~Curve() { }

		virtual void Draw(AxisType axes, DrawList* dl) = 0;

		v4 col;
		float thickness;
	};

	struct Parabola : public Curve
	{
		Parabola(std::vector<std::pair<v2, v2>> d, const v4& c, float t);

		// (x, y), (t, dist)
		std::vector<std::pair<v2, v2>> data;
		void Draw(AxisType axes, DrawList* dl) override;
	};

	struct CurveXY : public Curve
	{
		CurveXY(std::vector<v2> d, const v4& c, float t);

		std::vector<v2> data;
		void Draw(AxisType axes, DrawList* dl) override;
	};

	std::vector<Curve*> curves;
};

extern inline CurveManager& GetCurveManager() { static CurveManager m; return m; }