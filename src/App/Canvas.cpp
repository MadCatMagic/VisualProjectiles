#include "App/Canvas.h"
#include "App/Simulation.h"
#include "App/ControlNode.h"
#include "App/Ground.h"

#include "Engine/Console.h"
#include "Engine/Input.h"
#include "Engine/DrawList.h"

#include "BBox.h"

#include "imgui.h"

Canvas::~Canvas()
{
}

void Canvas::InitCanvas()
{
    drawList.SetConversionCallback([this](const v2& p) -> v2 { return this->ptcts(p); });
    drawList.InitColours();
}

// a lot of this code is taken from the ImGui canvas example
void Canvas::CreateWindow(std::vector<Simulation*>& sims, int window_N)
{
    ImGui::Begin(("Canvas " + std::to_string(window_N + 1)).c_str());
    if (ImGui::BeginMenu("Colours"))
    {
        for (int i = 0; i < NUM_DRAW_COLOURS; i++)
            ImGui::ColorEdit4(drawList.colours[i].name.c_str(), &drawList.colours[i].col.Value.x, ImGuiColorEditFlags_NoInputs);
        ImGui::EndMenu();
    }
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
        v2 shiftAmount = (previousWindowSize - canvasPixelSize) * scale.x * 0.5f;
        position += shiftAmount;
    }
    previousWindowSize = canvasPixelSize;

    v2 canvasBottomRight = canvasPixelPos + canvasPixelSize;

    // Draw border and background color
    ImGuiIO& io = ImGui::GetIO();
    drawList.dl = ImGui::GetWindowDrawList();
    drawList.convertPosition = false;
    drawList.scaleFactor = scale.x;
    drawList.RectFilled(canvasPixelPos, canvasBottomRight, DrawColour::Canvas_BG);
    drawList.Rect(canvasPixelPos, canvasBottomRight, DrawColour::Canvas_Edge);

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvasPixelSize.ImGui(), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool isHovered = ImGui::IsItemHovered(); // Hovered
    const bool isActive = ImGui::IsItemActive();   // Held
    const v2 mouseCanvasPos = ScreenToCanvas((v2)io.MousePos);
    const v2 mousePos = CanvasToPosition(mouseCanvasPos);

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

    // Pan
    if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        position.x -= io.MouseDelta.x * scale.x;
        position.y -= io.MouseDelta.y * scale.y;
    }

    // taken from LevelEditor\...\Editor.cpp
    if (isHovered && io.MouseWheel != 0.0f)
    {
        scalingLevel -= (int)io.MouseWheel;
        // clamp(zoomLevel, 0, 31) inclusive
        scalingLevel = scalingLevel >= 0 ? (scalingLevel < NUM_SCALING_LEVELS ? scalingLevel : NUM_SCALING_LEVELS - 1) : 0;
        // 1.1 ^ -15
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
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);

    if (ImGui::BeginPopup("context"))
    {
        if (ImGui::MenuItem("y/x", nullptr, axisType == AxisType::XY))
            axisType = AxisType::XY;
        if (ImGui::MenuItem("x/t", nullptr, axisType == AxisType::XT))
            axisType = AxisType::XT;
        if (ImGui::MenuItem("y/t", nullptr, axisType == AxisType::YT))
            axisType = AxisType::YT;

        ImGui::EndPopup();
    }

    // Draw grid + all lines in the canvas
    drawList.dl->PushClipRect((canvasPixelPos + 1.0f).ImGui(), (canvasBottomRight - 1.0f).ImGui(), true);

    float k = 1.0f;
    float limScalingValue = -10;
    if (scalingLevel > 18)
    {
        k = 100.0f;
        limScalingValue = 18;
    }
    else if (scalingLevel > 3)
    {
        k = 10.0f;
        limScalingValue = 5;
    }
    const v2 gridStep = scale.reciprocal() * k * 10;
    const v2 gridStepSmall = scale.reciprocal() * k;

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
        if (scalingLevel - 5 < limScalingValue)
            for (int dx = 1; dx < 10; dx++)
            {
                std::string t = std::to_string(lroundf(position.x + (x + dx * gridStepSmall.x) * scale.x) / pixelsPerUnit);
                drawList.Text(v2(
                    canvasPixelPos.x + x + dx * gridStepSmall.x - 6.5f * t.size() - 2,
                    canvasPixelPos.y - position.y / scale.y + 1
                ), DrawColour::TextFaded, t.c_str());
            }
        std::string t = std::to_string(lroundf(position.x + x * scale.x) / pixelsPerUnit);
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
        if (scalingLevel - 5 < limScalingValue)
            for (int dy = 1; dy < 10; dy++)
            {
                std::string t = std::to_string(-lroundf(position.y + (y + dy * gridStepSmall.y) * scale.y) / pixelsPerUnit);
                drawList.Text(v2(
                    canvasPixelPos.x - position.x / scale.x - 6.5f * t.size() - 2,
                    canvasPixelPos.y + y + dy * gridStepSmall.y + 1
                ), DrawColour::TextFaded, t.c_str());
            }
        std::string t = std::to_string(-lroundf(position.y + y * scale.y) / pixelsPerUnit);
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
        drawList.Text(v2(otherSide.x - 14 * scale.x, 5 * scale.x), DrawColour::Text, "x");
    else
        drawList.Text(v2(otherSide.x - 14 * scale.x, 5 * scale.x), DrawColour::Text, "t");

    if (axisType == AxisType::XT)
        drawList.Text(v2(-14 * scale.y, position.y + 7 * scale.y), DrawColour::Text, "x");
    else
        drawList.Text(v2(-14 * scale.y, position.y + 7 * scale.y), DrawColour::Text, "y");

    ImGui::PushFont(textLODs[scalingLevel]);
    
    drawList.mathsWorld = true;
    if (axisType == AxisType::XY)
        GetGround().Draw(&drawList, v2(), v2());
    // DRAW STUFF
    for (Simulation* sim : sims)
        if (sim->enabled)
            sim->Draw(&drawList, axisType);
    if (axisType == AxisType::XY)
        for (ControlNode* node : ControlNode::aliveNodes)
            if (node->draw)
                node->Draw(&drawList, scale.x);
    drawList.mathsWorld = false;

    for (ControlNode* node : ControlNode::aliveNodes)
        node->changedThisFrame = false;

    ImGui::PopFont();
    drawList.dl->PopClipRect();

    ImGui::End();
}

float Canvas::GetSFFromScalingLevel(int scaling)
{
    float z = MIN_SCALE;
    for (int i = 0; i < scaling; i++)
        z *= 1.2f;
    return z;
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

void Canvas::GenerateAllTextLODs()
{
    // Init
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    for (int i = 0; i < NUM_SCALING_LEVELS; i++)
        textLODs[i] = io.Fonts->AddFontFromFileTTF("res/fonts/Cousine-Regular.ttf", 12.0f / GetSFFromScalingLevel(i));
}
