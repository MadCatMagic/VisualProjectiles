#include "App/App.h"

#include "imgui.h"
#include "Engine/Console.h"
#include "App/JSON.h"

#include "App/SimulationFactory.h"
#include "App/Simulations/Projectiles.h"

#include "App/Ground.h"

#include "App/CurveManager.h"

void App::Initialize()
{
    RegisterProjectiles();

    //c.GenerateAllTextLODs();
    drawStyle.InitColours();
    
    AddCanvas();
    RegisterJSONCommands();
    
    ImGuiStyle* style = &ImGui::GetStyle();

    style->WindowPadding = ImVec2(15, 15);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    //style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    //style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    //style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    //style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    //style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    //style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
    //style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
    //style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    //style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

    //io.Fonts->AddFontFromFileTTF(XorStrA("C:\\Windows\\Fonts\\Ruda-Bold.ttf"), 12);
    //io.Fonts->AddFontFromFileTTF(XorStrA("C:\\Windows\\Fonts\\Ruda-Bold.ttf"), 10);
    //io.Fonts->AddFontFromFileTTF(XorStrA("C:\\Windows\\Fonts\\Ruda-Bold.ttf"), 14);
    //io.Fonts->AddFontFromFileTTF(XorStrA("C:\\Windows\\Fonts\\Ruda-Bold.ttf"), 18);
}

void App::Update()
{
}

void App::UI(struct ImGuiIO* io, double averageFrameTime, double lastFrameTime)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (ImGui::MenuItem("Console", NULL, Console::instance->enabled))
                Console::instance->enabled = !Console::instance->enabled;
            if (ImGui::MenuItem("Debug", NULL, showDebug))
                showDebug = !showDebug;
            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Add Canvas"))
        {
            AddCanvas();
        }

        if (ImGui::BeginMenu("New Sim"))
        {
            for (const std::string& name : GetSimulationFactory().Names())
                if (ImGui::MenuItem(name.c_str()))
                    AddSim(name, v2::zero);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
    
	ImGui::DockSpaceOverViewport();

    DebugWindow(io, lastFrameTime, averageFrameTime);

	ImGui::Begin("App");

    // time controls themselves
    ImGui::Checkbox("Enable time", &doCutoff);
    if (doCutoff)
    {
        ImGui::DragFloat("Time cutoff", &tCutoff, 0.1f);
        if (tCutoff < 0.0f)
            tCutoff = 0.0f;

        // allow playing or pausing of time
        if (playingTime)
            timePassed += io->DeltaTime;

        ImGui::BeginDisabled(playingTime);
        if (ImGui::Button("Play"))
        {
            timePassed = 0.0f;
            playingTime = true;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(!playingTime);
        if (ImGui::Button("Stop"))
            playingTime = false;
        ImGui::EndDisabled();
    }
    else
        playingTime = false;

    ImGui::Separator();

    ImGui::Checkbox("Disable Control Nodes", &disableControls);

    ImGui::DragFloat("g", &Simulation::gravity.y);

    static int angleUnitSelected = 1;
    const char* items[] = {"degrees", "radians"};
    if (ImGui::Combo("angle unit", &angleUnitSelected, items, 2))
        ControlVector::setRadOrDeg((bool)angleUnitSelected);

    ImGui::PushItemWidth(60.0f);
    ImGui::DragFloat("ground m", &GetGround().m, 0.1f); ImGui::SameLine();
    ImGui::DragFloat("ground c", &GetGround().c, 0.1f);
    ImGui::PopItemWidth();

    ImGui::Separator();

    for (int n = 0; n < sims.size(); n++)
    {
        ImGui::PushID(n);

        // have to do it like this so ImGui doesn't freak the hell out
        ImGui::SetNextItemOpen(simTabOpen[n]);
        // disgusting but necessary
        ImGui::AlignTextToFramePadding();
        bool drawTree = ImGui::TreeNodeEx((const void*)sims[n], ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick, sims[n]->name.c_str());

        // Our buttons are both drag sources and drag targets here!
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            // Set payload to carry the index of our item (could be anything)
            ImGui::SetDragDropPayload("SIM_DRAGGING", &n, sizeof(int));

            // Display preview (could be anything, e.g. when dragging an image we could decide to display
            // the filename and a small preview of the image, etc.)
            ImGui::Text("Swap");
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SIM_DRAGGING"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                int payload_n = *(const int*)payload->Data;
           
                // swap simulation positions
                Simulation* tmp = sims[n];
                sims[n] = sims[payload_n];
                sims[payload_n] = tmp;

                bool tmpb = simTabOpen[n];
                simTabOpen[n] = simTabOpen[payload_n];
                simTabOpen[payload_n] = tmpb;
            }
            ImGui::EndDragDropTarget();
        }
        else if (drawTree)
            simTabOpen[n] = true;
        else
            simTabOpen[n] = false;

        ImGui::SameLine();
        if (ImGui::Checkbox("enable", &sims[n]->enabled))
        {
            if (sims[n]->enabled)
                sims[n]->OnEnable();
            else
                sims[n]->OnDisable();
        }

        if (drawTree)
        {
            ImGui::BeginDisabled(!sims[n]->enabled);
            static char buf[64] = "";
            strcpy_s(buf, sims[n]->name.c_str());

            ImGui::ColorEdit3("colour", &sims[n]->colour.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::SameLine();
            if (ImGui::InputText("name", buf, 64))
                sims[n]->name = std::string(buf);

            if (ImGui::Button("Delete"))
            {
                delete sims[n];
                sims.erase(sims.begin() + n);
                simTabOpen.erase(simTabOpen.begin() + n);
            }
            else
            {
                ImGui::NewLine();
                sims[n]->DrawUI();
            }

            ImGui::EndDisabled();
            ImGui::TreePop();
        }

        ImGui::PopID();
        ImGui::Separator();
    }

	ImGui::End();


    // render curves and canvases
    float t = FLT_MAX;
    if (doCutoff)
    {
        if (playingTime) t = timePassed;
        else             t = tCutoff;
    }

    for (int i = 0; i < (int)canvases.size(); i++)
        canvases[i]->CreateWindow(i, disableControls, this, &drawStyle);

    // recalculate curves
    for (Simulation* sim : sims)
        if (sim->enabled)
            sim->Calculate();

    for (ControlNode* node : ControlNode::aliveNodes)
        node->changedThisFrame = false;

    Canvas* toDestroy = nullptr;
    int toDestroyI = 0;
    for (int i = 0; i < (int)canvases.size(); i++)
        if (!canvases[i]->CreateSims(sims, t, disableControls))
        {
            toDestroy = canvases[i];
            toDestroyI = i;
        }

    if (toDestroy != nullptr)
    {
        canvases.erase(canvases.begin() + toDestroyI);
        delete toDestroy;
    }

    // delete cached curves
    GetCurveManager().ClearCurves();
}

void App::Release()
{
    for (Simulation* sim : sims)
        delete sim;

    for (Canvas* canvas : canvases)
        delete canvas;
}

void App::AddSim(const std::string& name, const v2& position)
{
    sims.push_back(GetSimulationFactory().Build(name, position));
    simTabOpen.push_back(true);
}

void App::DebugWindow(ImGuiIO* io, double lastFrameTime, double averageFrameTime)
{
    frameTimeWindow[frameTimeI] = (float)lastFrameTime;
    averageTimeWindow[frameTimeI] = (float)averageFrameTime;
    frameTimeI = (++frameTimeI) % FRAME_TIME_MOVING_WINDOW_SIZE;

    if (!showDebug) return;
    ImGui::Begin("Debug", &showDebug);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
    ImGui::Text("Update average %.3f ms/frame (%.1f potential FPS)", averageFrameTime, 1000.0f / averageFrameTime);
    // draw graph
    ImGui::PlotHistogram("frame times", frameTimeWindow, FRAME_TIME_MOVING_WINDOW_SIZE, 0, 0, 0.0f, 10.0f, ImVec2(0.0f, 40.0f));
    ImGui::PlotHistogram("avg frame times", averageTimeWindow, FRAME_TIME_MOVING_WINDOW_SIZE, 0, 0, 0.0f, 10.0f, ImVec2(0.0f, 40.0f));

    if (ImGui::BeginMenu("Colours"))
    {
        for (int i = 0; i < NUM_DRAW_COLOURS; i++)
            ImGui::ColorEdit4(drawStyle.colours[i].name.c_str(), &drawStyle.colours[i].col.Value.x, ImGuiColorEditFlags_NoInputs);
        ImGui::EndMenu();
    }

    ImGui::End();
}

void App::AddCanvas()
{
    Canvas* canvas = new Canvas();
    canvas->InitCanvas();
    canvases.push_back(canvas);
}
