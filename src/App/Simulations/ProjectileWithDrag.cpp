#include "App/Simulations/ProjectileWithDrag.h"

#include "App/Ground.h"
#include "App/CurveManager.h"

#include "imgui.h"

ProjectileWithDrag::ProjectileWithDrag(const v2& position, const std::string& type)
	: Simulation(position, type)
{
	intersectXAxis.draw = false;
	intersectXAxis.style = ControlNode::Style::CrossDiagonal;
	intersectXAxis.colour = v4(1.0f);
	intersectXAxis.positionFixed = true;
}

JSONType ProjectileWithDrag::SaveState()
{
	std::unordered_map<std::string, JSONType> map = {
		{ "dt", dt },
		{ "usePhysicalObject", usePhysicalObject },
		{ "shape", (long)shape },
		{ "dragCoefficient", dragCoefficient },
		{ "airDensity", airDensity },
		{ "crossSectionalArea", crossSectionalArea },
		{ "mass", mass }
	};

	return { map };
}

void ProjectileWithDrag::LoadState(JSONType& state)
{
	dt = (float)state.obj["dt"].f;
	usePhysicalObject = state.obj["usePhysicalObject"].b;
	shape = (DragShape)state.obj["shape"].i;
	dragCoefficient = (float)state.obj["dragCoefficient"].f;
	airDensity = (float)state.obj["airDensity"].f;
	crossSectionalArea = (float)state.obj["crossSectionalArea"].f;
	mass = (float)state.obj["mass"].f;
}

void ProjectileWithDrag::OnDisable()
{
	startPos.draw = false;
	startVel.draw = false;
	intersectXAxis.draw = false;
}

void ProjectileWithDrag::OnEnable()
{
	startPos.draw = true;
	startVel.draw = true;
}

void ProjectileWithDrag::Calculate()
{
	std::vector<CurveManager::SignificantPoint> sigPoints;

	v2 p0 = startPos.getPosGlobal();
	v2 prevPos = p0;
	v2 vel = startVel.getPosLocal();
	float t = 0.0f;

	bool aboveGround = prevPos.y > 0.0f;
	distanceTravelled = 0.0f;

	std::vector<std::pair<v2, v2>> parabolaData;
	parabolaData.push_back({ prevPos, { } });

	float k = 0.5f * dragCoefficient * airDensity * crossSectionalArea / mass;
	bool ascending = vel.y >= 0.0f;
	for (int i = 0; i < 1000; i++)
	{
		v2 newPos = prevPos + vel * dt;
		if (ascending && newPos.y < prevPos.y)
		{
			ascending = false;
			sigPoints.push_back({ CurveManager::SignificantPoint::Type::Maximum, prevPos, v2(t, (p0 - prevPos).length()) });
		}
		
		float v = vel.length();
		v2 accel = v2(
			- vel.x * v * k,
			- gravity.y - vel.y * v * k
		);

		vel = vel + accel * dt;
		float newt = t + dt;
		float dist = (p0 - newPos).length();
		distanceTravelled += (newPos - prevPos).length();

		if (GetGround().AboveGround(prevPos))
		{
			IntersectionResult r = GetGround().TestIntersect(prevPos, newPos, dt);
			if (r.intersected)
			{
				newPos = r.position;
				newt = t + r.dt;
				dist = (p0 - newPos).length();
				intersectXAxis.draw = true;
				intersectXAxis.setPosGlobal(newPos);

				parabolaData.push_back({ newPos, v2(newt, dist) });
				GetCurveManager().ParabolaData(parabolaData, sigPoints, colour);
				return;
			}
		}

		parabolaData.push_back({ newPos, v2(newt, dist) });

		t = newt;
		prevPos = newPos;
	}

	// dispatch to be drawn
	GetCurveManager().ParabolaData(parabolaData, sigPoints, colour);
	intersectXAxis.draw = false;
}

void ProjectileWithDrag::DrawUI()
{
	ImGui::Text(("Distance travelled by projectile: " + ftos(distanceTravelled)).c_str());
	ImGui::SliderFloat("dt", &dt, 0.005f, 10.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
	ImGui::NewLine();

	ImGui::Checkbox("use physical object", &usePhysicalObject);
	ImGui::BeginDisabled(usePhysicalObject);
	ImGui::SliderFloat("drag coefficient", &dragCoefficient, 0.0f, 2.5f);
	ImGui::EndDisabled();

	// pretty shady/RADICAL
	if (usePhysicalObject)
		if (ImGui::ListBox("shape", (int*)(&shape), dragNames, 9))
			switch (shape)
			{
			case DragShape::Sphere:				 { dragCoefficient = 0.47f; break; }
			case DragShape::HalfSphere:			 { dragCoefficient = 0.42f; break; }
			case DragShape::Cone:				 { dragCoefficient = 0.50f; break; }
			case DragShape::Cube:				 { dragCoefficient = 1.05f; break; }
			case DragShape::AngledCube:			 { dragCoefficient = 0.80f; break; }
			case DragShape::LongCylinder:		 { dragCoefficient = 0.82f; break; }
			case DragShape::ShortCylinder:		 { dragCoefficient = 1.15f; break; }
			case DragShape::StreamlinedBody:	 { dragCoefficient = 0.04f; break; }
			case DragShape::StreamlinedHalfBody: { dragCoefficient = 0.09f; break; }
			}

	ImGui::NewLine();
	ImGui::InputFloat("air density (kgm^-3)", &airDensity);
	ImGui::InputFloat("cross-sectional area (m^2)", &crossSectionalArea);
	ImGui::InputFloat("mass (kg)", &mass);
}
