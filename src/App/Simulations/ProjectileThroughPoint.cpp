#include "App/Simulations/ProjectileThroughPoint.h"

#include "App/Ground.h"
#include "Engine/DrawList.h"

ProjectileThroughPoint::ProjectileThroughPoint()
{
	controlPoint.style = ControlNode::Style::Circross;
	controlPoint.colour = v4(1.0f, 0.6f, 0.2f, 0.9f);

	startVel.positionFixed = true;
	startVel.colour = v4(0.0f, 0.0f, 0.0f, 0.0f);
	vel2.positionFixed = true;
	vel2.colour = v4(0.0f, 0.0f, 0.0f, 0.0f);
	vel2.root = &startPos;

	maximum.style = ControlNode::Style::Cross;
	maximum.colour = v4(1.0f);
}

void ProjectileThroughPoint::OnDisable()
{
	vel2.draw = false;
	startPos.draw = false;
	startVel.draw = false;
	controlPoint.draw = false;
	maximum.draw = false;
}

void ProjectileThroughPoint::OnEnable()
{
	vel2.draw = true;
	startPos.draw = true;
	startVel.draw = true;
	controlPoint.draw = true;
}

void ProjectileThroughPoint::Draw(DrawList* drawList, AxisType axes)
{
	v2 p0 = startPos.getPosGlobal();
	v2 p1 = controlPoint.getPosGlobal();

	if (GetGround().BelowGround(p0))
	{
		maximum.draw = false;
		return;
	}

	// work out the possible u based on our maximum
	v2 max = maximum.getPosGlobal();
	float k = 2.0f * (max.y - p0.y);
	float u = sqrtf(gravity.y * ((max.x - p0.x) * (max.x - p0.x) / k + k));

	// compare it to minimum, reject if too low
	v2 pd = p1 - p0;
	float mag = sqrtf(gravity.y * (pd.y + pd.length()));
	if (u <= mag || isnan(u))
		u = mag;

	// calculate velocity vector
	float theta1, theta2;
	{
		float a = gravity.y / (2.0f * u * u) * pd.x * pd.x;
		float b = -pd.x;
		float c = pd.y + gravity.y * pd.x * pd.x / (2.0f * u * u);
		theta1 = atanf((-b + sqrtf(std::max(0.0f, b * b - 4.0f * a * c))) / (2.0f * a));
		theta2 = atanf((-b - sqrtf(std::max(0.0f, b * b - 4.0f * a * c))) / (2.0f * a));
	}
	// protec
	if (isnan(theta1) || isnan(theta2))
		return;

	// draw possible range of maximums
	v2 pmp;
	bool first = true;
	for (float logt = 0.0f; logt < 5.0f; logt += 0.05f)
	{
		float t = mag + powf(10.0f, logt) - 1.0f;
		float kv = t * t + sqrtf(std::max(t * t * (t * t - 2.0f * gravity.y * pd.y) - gravity.y * gravity.y * pd.x * pd.x, 0.0f));
		float h = gravity.y * gravity.y * pd.x * pd.x + kv * kv;
		v2 mp = v2(pd.x / h, kv / (2.0f * gravity.y * h)) * (t * t * kv) + p0;
		if (!first)
			drawList->Line(pmp, mp, DrawColour::Canvas_GridLinesHeavy);
		else
			first = false;
		pmp = mp;
	}

	v2 v0 = v2(cosf(theta1), sinf(theta1)) * u;
	v2 v01 = v2(cosf(theta2), sinf(theta2)) * u;
	startVel.setPosLocal(v0);
	vel2.setPosLocal(v01);

	// (x/g)(y+sqrt(y^2+2gh))
	float R = v0.x / gravity.y * (v0.y + sqrtf(v0.y * v0.y + 2.0f * gravity.y * p0.y));
	auto pair = Parabola(drawList, p0, v0, R, true, axes, colour);

	float R2 = v01.x / gravity.y * (v01.y + sqrtf(v01.y * v01.y + 2.0f * gravity.y * p0.y));
	Parabola(drawList, p0, v01, R2, true, axes, colour * 0.5f);

	// reset maximum point on normal curve
	max = v2(p0.x + v0.x * v0.y / gravity.y, p0.y + v0.y * v0.y / (2.0f * gravity.y));
	if (max.x < p0.x)
		maximum.draw = false;
	else
	{
		maximum.draw = true;
		maximum.setPosGlobal(max);
	}
}

void ProjectileThroughPoint::DrawUI()
{
}