#include "App/Canvas.h"
#include "App/Simulation.h"
#include "App/ControlNode.h"
#include "App/Ground.h"
#include "App/CurveManager.h"

#include "Engine/Console.h"
#include "Engine/Input.h"
#include "Engine/DrawList.h"

#include "BBox.h"

#include "imgui.h"

#include "App/App.h"
#include "App/SimulationFactory.h"

#include <sstream>
#include <iomanip>

Canvas::Canvas(JSONType& state)
{
    scalingLevel = v2i(
        (int)state.obj["scalingLevelx"].i,
        (int)state.obj["scalingLevely"].i
    );
    auto& pv = state.obj["position"].arr;
    position = v2((float)pv[0].f, (float)pv[1].f);
    
    axisType = (AxisType)state.obj["axisType"].i;

    scale = GetSFFromScalingLevel(scalingLevel);
}

Canvas::~Canvas()
{
}

void Canvas::InitCanvas()
{
    drawList.SetConversionCallback([this](const v2& p) -> v2 { return this->ptcts(p); });
}

const std::string axisTypeToString[4]
{
    "y/x", "x/t", "y/t", "|x-x0|/t"
};

// a lot of this code is taken from the ImGui canvas example
void Canvas::CreateWindow(int window_N, bool disableControls, App* app, DrawStyle* drawStyle)
{
    // necessary so can have dynamic window title
    std::string title = "Canvas " + std::to_string(window_N + 1);
    title += " - " + axisTypeToString[(int)axisType];
    title += "###Canvas " + std::to_string(window_N + 1);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(-1, -1));
    ImGui::Begin(title.c_str(), &shouldStayOpen);
    ImGui::PopStyleVar();

    //if (ImGui::BeginMenu("Colours"))
    //{
    //    for (int i = 0; i < NUM_DRAW_COLOURS; i++)
    //        ImGui::ColorEdit4(drawList.colours[i].name.c_str(), &drawList.colours[i].col.Value.x, ImGuiColorEditFlags_NoInputs);
    //    ImGui::EndMenu();
    //}
    // ImGui::InputFloat2("position", &position.x);

    // Using InvisibleButton() as a convenience 
    // 1) it will advance the layout cursor and 
    // 2) allows us to use IsItemHovered()/IsItemActive()
    canvasPixelPos = (v2)ImGui::GetCursorScreenPos();
    canvasPixelSize = ImGui::GetContentRegionAvail();
    if (canvasPixelSize.x < 50.0f) canvasPixelSize.x = 50.0f;
    if (canvasPixelSize.y < 50.0f) canvasPixelSize.y = 50.0f;

    if (previousWindowSize != v2() && previousWindowSize != canvasPixelSize)
    {
        v2 shiftAmount = (previousWindowSize - canvasPixelSize).scale(scale) * 0.5f;
        position += shiftAmount;
    }
    previousWindowSize = canvasPixelSize;

    v2 canvasBottomRight = canvasPixelPos + canvasPixelSize;

    // Draw border and background color
    ImGuiIO& io = ImGui::GetIO();
    drawList.dl = ImGui::GetWindowDrawList();
    drawList.style = drawStyle;
    drawList.convertPosition = false;
    drawList.scaleFactor = scale;
    drawList.RectFilled(canvasPixelPos, canvasBottomRight, DrawColour::Canvas_BG);
    drawList.Rect(canvasPixelPos, canvasBottomRight, DrawColour::Canvas_Edge);

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvasPixelSize.ImGui(), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    const bool isHovered = ImGui::IsItemHovered(); // Hovered
    const bool isActive = ImGui::IsItemActive();   // Held
    const v2 mouseCanvasPos = ScreenToCanvas((v2)io.MousePos);
    const v2 mousePos = CanvasToPosition(mouseCanvasPos);

    if (!disableControls)
    {
        // start dragging control nodes
        if (isActive && !draggingControlNode && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            float bestDist = FLT_MAX;
            ControlNode* closest = nullptr;
            for (ControlNode* node : ControlNode::aliveNodes)
            {
                if (node->positionFixed)
                    continue;
                float dist = mousePos.scale(v2(0.1f, -0.1f)).distanceTo(node->getPosGlobal());
                if (dist <= 2.0f * scale.x && dist < bestDist)
                {
                    bestDist = dist;
                    closest = node;
                }
            }
            if (closest != nullptr)
            {
                draggingControlNode = true;
                selectedControlNode = closest;
                originalCNPosition = closest->getPosGlobal();
            }
        }

        // drag control nodes
        if (draggingControlNode)
        {
            if (isActive)
            {
                // dealing in real world coords here
                cumulativeCNOffset.x += io.MouseDelta.x * 0.1f * scale.x;
                cumulativeCNOffset.y -= io.MouseDelta.y * 0.1f * scale.y;
                if (Input::GetKey(Input::Key::LSHIFT))
                    selectedControlNode->setPosGlobal(originalCNPosition + cumulativeCNOffset);
                else if (Input::GetKey(Input::Key::LCONTROL))
                    // round
                    selectedControlNode->setPosGlobal((v2)v2i(originalCNPosition + cumulativeCNOffset + 0.5f));
                else
                    selectedControlNode->setPosGlobal((v2)v2i((originalCNPosition + cumulativeCNOffset) * 10.0f + 0.5f) * 0.1f);
                selectedControlNode->changedThisFrame = true;
            }
            else
            {
                draggingControlNode = false;
                selectedControlNode = nullptr;
                cumulativeCNOffset = v2::zero;
            }
        }
    }
    else
    {
        draggingControlNode = false;
        selectedControlNode = nullptr;
        cumulativeCNOffset = v2::zero;
    }

    // Pan
    if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        position.x -= io.MouseDelta.x * scale.x;
        position.y -= io.MouseDelta.y * scale.y;
    }

    // taken from LevelEditor\...\Editor.cpp
    if (isHovered && io.MouseWheel != 0.0f)
    {
        // scaling is in both axes now
        // **ADVANCED**
        if ( !(
            !Input::GetKey(Input::Key::LSHIFT) && !Input::GetKey(Input::Key::LCONTROL) &&
            (scalingLevel.x == -51 || scalingLevel.y == -51) && io.MouseWheel > 0.0f
        ) && !(
            !Input::GetKey(Input::Key::LSHIFT) && !Input::GetKey(Input::Key::LCONTROL) &&
            (scalingLevel.x == 31 || scalingLevel.y == 31) && io.MouseWheel < 0.0f
        ))
        {
            if (!Input::GetKey(Input::Key::LSHIFT))
                scalingLevel.x -= (int)io.MouseWheel;
            if (!Input::GetKey(Input::Key::LCONTROL))
                scalingLevel.y -= (int)io.MouseWheel;
        }

        // clamp to -51
        scalingLevel = v2i(
            std::min(std::max(scalingLevel.x, -51), 31),
            std::min(std::max(scalingLevel.y, -51), 31)
        );

        v2 prevScale = scale;
        scale = GetSFFromScalingLevel(scalingLevel);
        // position + (mousePosBefore = canvasPos * scaleBefore + position) - (mousePosAfter = canvasPos * scaleAfter + position)
        // position + canvasPos * (scaleBefore - scaleAfter)
        // somehow it has to be negative... I hate you linear algebra!!!
        position -= mouseCanvasPos.scale(prevScale - scale);
    }

    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f)
    {
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
        if (io.MouseClicked[1])
            contextMenuClickPos = mousePos;
    }

    if (ImGui::BeginPopup("context"))
    {
        ImGui::SliderFloat("fv", &fv, -20, 20);

        if (ImGui::BeginMenu("Add Node"))
        {
            for (const std::string& name : GetSimulationFactory().Names())
                if (ImGui::Selectable(name.c_str()))
                    app->AddSim(name, (contextMenuClickPos * 0.1f).scale(v2(1.0f, -1.0f)));
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("y/x", nullptr, axisType == AxisType::XY))
            axisType = AxisType::XY;
        if (ImGui::MenuItem("x/t", nullptr, axisType == AxisType::XT))
            axisType = AxisType::XT;
        if (ImGui::MenuItem("y/t", nullptr, axisType == AxisType::YT))
            axisType = AxisType::YT;
        if (ImGui::MenuItem("|p0-p|/t", nullptr, axisType == AxisType::DistT))
            axisType = AxisType::DistT;

        ImGui::EndPopup();
    }

    // Draw grid + all lines in the canvas
    drawList.dl->PushClipRect((canvasPixelPos + 1.0f).ImGui(), (canvasBottomRight - 1.0f).ImGui(), true);

    const v2 kpart = v2((scalingLevel.x + 10) * 0.07918124604f, (scalingLevel.y + 10) * 0.07918124604f);
    const v2 k = v2(
        powf(10.0f, (float)(int)kpart.x + (kpart.x < 0.0f ? 0.0f : 1.0f)),
        powf(10.0f, (float)(int)kpart.y + (kpart.y < 0.0f ? 0.0f : 1.0f))
    );
    const v2 limScalingValue = (v2)(v2i)kpart / 0.07918124604f - v2(scalingLevel.x < -10.0f ? 20.2f : 4.5f, scalingLevel.y < -10.0f ? 20.2f : 4.5f);
    
    const v2 gridStep = scale.reciprocal().scale(k) * 10.0f;
    const v2 gridStepSmall = scale.reciprocal().scale(k);

    // draw lines, axes numbers
    // horrible code, dont look
    for (float x = fmodf(-position.x / scale.x, gridStep.x) - gridStep.x; x < canvasPixelSize.x; x += gridStep.x)
    {
        drawList.Line(v2(canvasPixelPos.x + x, canvasPixelPos.y), v2(canvasPixelPos.x + x, canvasBottomRight.y), DrawColour::Canvas_GridLinesHeavy);
        for (int dx = 1; dx < 10; dx++)
            drawList.Line(
                ImVec2(canvasPixelPos.x + x + dx * gridStepSmall.x, canvasPixelPos.y), 
                ImVec2(canvasPixelPos.x + x + dx * gridStepSmall.x, canvasBottomRight.y), DrawColour::Canvas_GridLinesLight);

        // axes numbers
        // this is SERIOUSLY FUCKED
        if (scalingLevel.x < limScalingValue.x)
            for (int dx = 1; dx < 10; dx++)
            {
                std::string t = pprint(position.x + (x + dx * gridStepSmall.x) * scale.x, k.x);
                drawList.Text(v2(
                    canvasPixelPos.x + x + dx * gridStepSmall.x - 6.5f * t.size() - 2,
                    canvasPixelPos.y - position.y / scale.y + 1
                ), DrawColour::TextFaded, t.c_str());
            }
        std::string t = pprint(position.x + x * scale.x, k.x);
        drawList.Text(v2(
            canvasPixelPos.x + x - 6.5f * t.size() - 2, 
            canvasPixelPos.y - position.y / scale.y + 1
        ), DrawColour::TextFaded, t.c_str());
    }

    for (float y = fmodf(-position.y / scale.y, gridStep.y) - gridStep.y; y < canvasPixelSize.y; y += gridStep.y)
    {
        drawList.Line(v2(canvasPixelPos.x, canvasPixelPos.y + y), v2(canvasBottomRight.x, canvasPixelPos.y + y), DrawColour::Canvas_GridLinesHeavy);
        for (int dy = 1; dy < 10; dy++)
            drawList.Line(
                v2(canvasPixelPos.x, canvasPixelPos.y + y + dy * gridStepSmall.y),
                v2(canvasBottomRight.x, canvasPixelPos.y + y + dy * gridStepSmall.y), DrawColour::Canvas_GridLinesLight);
        
        // axes numbers
        if (scalingLevel.y < limScalingValue.y)
            for (int dy = 1; dy < 10; dy++)
            {
                std::string t = pprint(-(position.y + (y + dy * gridStepSmall.y) * scale.y), k.y);
                drawList.Text(v2(
                    canvasPixelPos.x - position.x / scale.x - 6.5f * t.size() - 2,
                    canvasPixelPos.y + y + dy * gridStepSmall.y + 1
                ), DrawColour::TextFaded, t.c_str());
            }
        std::string t = pprint(-(position.y + y * scale.y), k.y);
        drawList.Text(v2(
            canvasPixelPos.x - position.x / scale.x - 6.5f * t.size() - 2,
            canvasPixelPos.y + y + 1
        ), DrawColour::TextFaded, t.c_str());
    }

    // draw axes
    drawList.convertPosition = true;
    v2 otherSide = position + canvasPixelSize.scale(scale);
    drawList.Line(v2(position.x, 0), v2(otherSide.x, 0), DrawColour::Canvas_Axes, 2.0f);
    drawList.Line(v2(0, position.y), v2(0, otherSide.y), DrawColour::Canvas_Axes, 2.0f);
    // horrificly specific values :(
    if (axisType == AxisType::XY)
        drawList.Text(v2(otherSide.x - 14 * scale.x, 5 * scale.y), DrawColour::Text, "x");
    else
        drawList.Text(v2(otherSide.x - 14 * scale.x, 5 * scale.y), DrawColour::Text, "t");

    if (axisType == AxisType::XT)
        drawList.Text(v2(-14 * scale.x, position.y + 7 * scale.y), DrawColour::Text, "x");
    else if (axisType == AxisType::YT || axisType == AxisType::XY)
        drawList.Text(v2(-14 * scale.x, position.y + 7 * scale.y), DrawColour::Text, "y");
    else
        drawList.Text(v2(-63 * scale.x, position.y + 7 * scale.y), DrawColour::Text, "|p0 - p|");

    drawList.dl->PopClipRect();
}

bool Canvas::CreateSims(std::vector<class Simulation*>& sims, float tCutoff, bool disableControls)
{
    v2 canvasBottomRight = canvasPixelPos + canvasPixelSize;
    drawList.dl->PushClipRect((canvasPixelPos + 1.0f).ImGui(), (canvasBottomRight - 1.0f).ImGui(), true);

    drawList.mathsWorld = true;
    if (axisType == AxisType::XY)
        GetGround().Draw(&drawList, bbox2(stctp(canvasPixelPos).scale(v2(1.0f, -1.0f)) * 0.1f, stctp(canvasBottomRight).scale(v2(1.0f, -1.0f)) * 0.1f));

    // DRAW STUFF
    //for (Simulation* sim : sims)
    //    if (sim->enabled)
    //        sim->Draw(&drawList, axisType);
    GetCurveManager().DrawCurves(axisType, &drawList, tCutoff);

    if (!disableControls)
    {
        if (axisType == AxisType::XY)
            for (ControlNode* node : ControlNode::aliveNodes)
                if (node->draw)
                    node->Draw(&drawList, scale);
    }
    drawList.mathsWorld = false;

    drawList.dl->PopClipRect();

    ImGui::End();

    return shouldStayOpen;
}

v2 Canvas::GetSFFromScalingLevel(const v2i& scaling)
{
    float xs = 1.0f;
    for (int i = 0; i < scaling.x; i++)
        xs *= 1.2f;
    for (int i = 0; i > scaling.x; i--)
        xs /= 1.2f;
    float ys = 1.0f;
    for (int i = 0; i < scaling.y; i++)
        ys *= 1.2f;
    for (int i = 0; i > scaling.y; i--)
        ys /= 1.2f;
    return v2(xs, ys);
}

JSONType Canvas::SaveState()
{
    std::unordered_map<std::string, JSONType> map = {
        { "scalingLevelx", (long)scalingLevel.x },
        { "scalingLevely", (long)scalingLevel.y },
        { "position", position },
        { "axisType", (long)axisType }
    };

    return { map };
}

// real fucky
std::string Canvas::pprint(float n, float s)
{
    float nn = n / pixelsPerUnit;
    float adjusted = (nn >= -0.5f) ? nn + 0.5f : nn - 0.5f;
    if (abs(nn - (float)(long)adjusted) < s * 0.0001f)
        return std::to_string((long)adjusted);
    
    for (float k = 10.0f; k <= 1000.0f; k *= 10.0f)
    {
        float kn = nn * k + ((nn * k >= -0.5f) ? 0.5f : -0.5f);
        if (abs(nn * k - (float)(long)kn) < 0.0001f * k)
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision((std::streamsize)log10f(k + 1.0f)) << nn;
            return ss.str();
        }
    }

    return std::to_string(nn);
}

v2 Canvas::ScreenToCanvas(const v2& pos) const // c = s + p
{
    return canvasPixelPos - pos;
}

v2 Canvas::CanvasToScreen(const v2& pos) const // s = c - p
{
    return canvasPixelPos - pos;
}

v2 Canvas::CanvasToPosition(const v2& pos) const // position = offset - canvas * scale
{
    return position - pos.scale(scale);
}

v2 Canvas::PositionToCanvas(const v2& pos) const // canvas = (offset - position) / scale
{
    return (position - pos).scale(scale.reciprocal());
}