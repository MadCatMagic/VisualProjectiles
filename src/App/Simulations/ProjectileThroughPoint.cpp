#include "App/Simulations/ProjectileThroughPoint.h"

#include "App/Ground.h"
#include "App/CurveManager.h"

#include "imgui.h"

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
	shyMaximum.style = ControlNode::Style::Cross;
	shyMaximum.colour = v4(0.6f);
	shyMaximum.positionFixed = true;
}

void ProjectileThroughPoint::OnDisable()
{
	vel2.draw = false;
	startPos.draw = false;
	startVel.draw = false;
	controlPoint.draw = false;
	maximum.draw = false;
	shyMaximum.draw = false;
}

void ProjectileThroughPoint::OnEnable()
{
	vel2.draw = true;
	startPos.draw = true;
	startVel.draw = true;
	controlPoint.draw = true;
}

void ProjectileThroughPoint::Calculate()
{
	v2 p0 = startPos.getPosGlobal();
	v2 p1 = controlPoint.getPosGlobal();

	if (GetGround().BelowGround(p0))
	{
		maximum.draw = false;
		shyMaximum.draw = false;
		return;
	}

	// work out the possible u based on our maximum
	v2 max = maximum.getPosGlobal();
	float k = 2.0f * (max.y - p0.y);
	float u = sqrtf(gravity.y * ((max.x - p0.x) * (max.x - p0.x) / k + k));

	v2 pd = p1 - p0;

	// set u from the angle if it is the control point that is moving
	if (controlPoint.changedThisFrame)
	{
		v2 v0 = startVel.getPosLocal();
		u = sqrtf(
			(gravity.y * pd.x * pd.x * (v0.length2() / (v0.x * v0.x))) /
			(2.0f * (pd.x * v0.y / v0.x - pd.y))
		);
	}

	// if maximum node is below the theoretical lowest maximum, don't care
	float mag = sqrtf(gravity.y * (pd.y + pd.length()));
	float vmmyp = pd.y + pd.length();
	float vmmy = vmmyp * vmmyp / (pd.x * pd.x + vmmyp * vmmyp);
	float lowestMax = p0.y + mag * mag * vmmy / (2.0f * gravity.y);

	// compare it to minimum, reject if too low
	if (abs(u - mag) < 0.02f)
		lockMinU = true;
	if (u <= mag || isnan(u) || lowestMax > max.y || (lockMinU && !maximum.changedThisFrame))
	{
		u = mag;
		if (controlPoint.changedThisFrame)
			lockMinU = true;
	}
	else
		lockMinU = false;

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
	if (drawMaximumPossibilitiesLine)
	{
		std::vector<v2> drawArr;

		// first up
		for (float logt = 0.0f; logt < 4.0f; logt += 0.1f)
		{
			float t = mag + powf(2.0f, logt * logt) - 1.0f;
			float kv = t * t + 
				sqrtf(std::max(t * t * (t * t - 2.0f * gravity.y * pd.y) - gravity.y * gravity.y * pd.x * pd.x, 0.0f));
			float h = gravity.y * gravity.y * pd.x * pd.x + kv * kv;
			v2 mp = v2(pd.x / h, kv / (2.0f * gravity.y * h)) * (t * t * kv) + p0;
			drawArr.push_back(mp);
		}
		GetCurveManager().CurveXYData(drawArr, v4(0.4f, 0.4f, 0.4f, 0.6f));

		drawArr.clear();
		// then down
		for (float logt = 0.0f; logt < 3.0f; logt += 0.1f)
		{
			float t = mag + powf(2.0f, logt * logt) - 1.0f;
			float kv = t * t -
				sqrtf(std::max(t * t * (t * t - 2.0f * gravity.y * pd.y) - gravity.y * gravity.y * pd.x * pd.x, 0.0f));
			float h = gravity.y * gravity.y * pd.x * pd.x + kv * kv;
			v2 mp = v2(pd.x / h, kv / (2.0f * gravity.y * h)) * (t * t * kv) + p0;
			drawArr.push_back(mp);
		}
		GetCurveManager().CurveXYData(drawArr, v4(0.4f, 0.4f, 0.4f, 0.6f));
	}

	v2 v0 = v2(cosf(theta1), sinf(theta1)) * u;
	v2 v01 = v2(cosf(theta2), sinf(theta2)) * u;
	startVel.setPosLocal(v0);
	vel2.setPosLocal(v01);

	// (x/g)(y+sqrt(y^2+2gh))
	float R = v0.x / gravity.y * (v0.y + sqrtf(v0.y * v0.y + 2.0f * gravity.y * p0.y));
	auto pair = Parabola(p0, v0, R, colour, ParabolaFlag_GroundCheck);

	float R2 = v01.x / gravity.y * (v01.y + sqrtf(v01.y * v01.y + 2.0f * gravity.y * p0.y));
	Parabola(p0, v01, R2, colour * 0.5f, ParabolaFlag_GroundCheck);

	// reset maximum point on normal curve
	max = v2(p0.x + v0.x * v0.y / gravity.y, p0.y + v0.y * v0.y / (2.0f * gravity.y));
	if (max.x < p0.x)
		maximum.draw = false;
	else
	{
		maximum.draw = true;
		maximum.setPosGlobal(max);
	}

	v2 shyMax = v2(p0.x + v01.x * v01.y / gravity.y, p0.y + v01.y * v01.y / (2.0f * gravity.y));
	if (shyMax.x < p0.x)
		shyMaximum.draw = false;
	else
	{
		shyMaximum.draw = true;
		shyMaximum.setPosGlobal(shyMax);
	}
}

void ProjectileThroughPoint::DrawUI()
{
	//ImGui::Checkbox("Lock to Minimum u", &lockMinU);
	//if (!lockMinU)
	ImGui::Checkbox("Draw Line of Possible Maximums", &drawMaximumPossibilitiesLine);
	if (drawMaximumPossibilitiesLine)
		ImGui::Checkbox("Switch Maximum", &maximumPossibilitiesLineLower);
}