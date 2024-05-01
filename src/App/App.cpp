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
    
    AddCanvas();
    RegisterJSONCommands();
    
    LoadStyle();
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

        if (ImGui::MenuItem("Save"))
            SaveState("goon.state");
        if (ImGui::MenuItem("Load"))
            LoadState("goon.state");

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

    int angleUnitSelected = (int)useRadians;
    if (ImGui::RadioButton("degrees", &angleUnitSelected, 0))
        ControlVector::setRadOrDeg((bool)angleUnitSelected);
    ImGui::SameLine();
    if (ImGui::RadioButton("radians", &angleUnitSelected, 1))
        ControlVector::setRadOrDeg((bool)angleUnitSelected);
    useRadians = (bool)angleUnitSelected;

    GetGround().UI();

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

void App::LoadState(const std::string& filename)
{
    JSONConverter converter;
    auto decodeResult = converter.DecodeFile(filename);
    if (!decodeResult.second)
    {
        Console::LogErr("FAILED TO DECODE FROM FILENAME `" + filename + "`.");
        return;
    }
    // clear out existing data
    for (Simulation* sim : sims)
        delete sim;
    sims.clear();
    for (Canvas* canvas : canvases)
        delete canvas;
    canvases.clear();
    simTabOpen.clear();

    auto& state = decodeResult.first;

    // load all the app data
    showDebug = state["showDebug"].b;

    doCutoff = state["doCutoff"].b;
    tCutoff = (float)state["tCutoff"].f;

    disableControls = state["disableControls"].b;

    Simulation::gravity = v2(0.0f, (float)state["gravity"].f);
    useRadians = state["useRadians"].b;
    ControlVector::setRadOrDeg(useRadians);

    for (JSONType& c : state["canvases"].arr)
    {
        Canvas* canvas = new Canvas(c);
        canvas->InitCanvas();
        canvases.push_back(canvas);
    }

    for (JSONType& s : state["sims"].arr)
    {
        std::string name = s.obj["type"].s;

        Simulation* sim = GetSimulationFactory().Build(name, v2(0.0f, 0.0f));
        sim->_LoadState(s);
        sims.push_back(sim);
    }

    for (JSONType& sto : state["sims"].arr)
        simTabOpen.push_back(sto.b);

    ControlVector::Root();
}

void App::SaveState(const std::string& filename)
{
    std::vector<JSONType> canvasData;
    for (Canvas* c : canvases)
        canvasData.push_back(c->SaveState());

    std::vector<JSONType> simData;
    for (Simulation* s : sims)
        simData.push_back(s->_SaveState());

    std::vector<JSONType> simTabData;
    for (bool sto : simTabOpen)
        simTabData.push_back(sto);

    std::unordered_map<std::string, JSONType> map = {
        { "showDebug", showDebug },

        { "doCutoff", doCutoff },
        { "tCutoff", tCutoff },

        { "disableControls", disableControls },

        { "gravity", Simulation::gravity.y },
        { "useRadians", useRadians },

        { "canvases", canvasData },
        { "sims", simData },
        { "simTabOpen", simTabData }
    };

    JSONConverter converter;
    JSONType t = { map };
    converter.WriteFile(filename, t, false);
}

#pragma region STYLE

void App::LoadStyle()
{

    //c.GenerateAllTextLODs();
    drawStyle.InitColours();

    ImGuiStyle* style = &ImGui::GetStyle();
    style->Alpha = 1.0f;
    style->DisabledAlpha = 0.1000000014901161f;
    style->WindowPadding = ImVec2(8.0f, 8.0f);
    style->WindowRounding = 10.0f;
    style->WindowBorderSize = 0.0f;
    style->WindowMinSize = ImVec2(30.0f, 30.0f);
    style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style->WindowMenuButtonPosition = ImGuiDir_Right;
    style->ChildRounding = 5.0f;
    style->ChildBorderSize = 1.0f;
    style->PopupRounding = 10.0f;
    style->PopupBorderSize = 0.0f;
    style->FramePadding = ImVec2(5.0f, 5.0f);
    style->FrameRounding = 5.0f;
    style->FrameBorderSize = 0.0f;
    style->ItemSpacing = ImVec2(5.0f, 5.0f);
    style->ItemInnerSpacing = ImVec2(5.0f, 5.0f);
    style->CellPadding = ImVec2(4.0f, 4.0f);
    style->IndentSpacing = 5.0f;
    style->ColumnsMinSpacing = 5.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 15.0f;
    style->GrabRounding = 5.0f;
    style->TabRounding = 5.0f;
    style->TabBorderSize = 0.0f;
    style->TabMinWidthForCloseButton = 0.0f;
    style->ColorButtonPosition = ImGuiDir_Right;
    style->ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style->SelectableTextAlign = ImVec2(0.0f, 0.0f);

    // what the FUCK
    style->Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style->Colors[ImGuiCol_ChildBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549295425415f, 0.5529412031173706f, 1.0f, 1.0f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
    style->Colors[ImGuiCol_Separator] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
    style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
    style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 1.0f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
    style->Colors[ImGuiCol_Tab] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style->Colors[ImGuiCol_TabHovered] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style->Colors[ImGuiCol_TabActive] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
    style->Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2901960909366608f, 0.5960784554481506f, 1.0f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.9960784316062927f, 0.4745098054409027f, 0.6980392336845398f, 1.0f);
    style->Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style->Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
    style->Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    style->Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
    style->Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2352941185235977f, 0.2156862765550613f, 0.5960784554481506f, 1.0f);
    style->Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style->Colors[ImGuiCol_NavHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
    style->Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
    style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);

    //io.Fonts->AddFontFromFileTTF(XorStrA("C:\\Windows\\Fonts\\Ruda-Bold.ttf"), 12);
    //io.Fonts->AddFontFromFileTTF(XorStrA("C:\\Windows\\Fonts\\Ruda-Bold.ttf"), 10);
    //io.Fonts->AddFontFromFileTTF(XorStrA("C:\\Windows\\Fonts\\Ruda-Bold.ttf"), 14);
    //io.Fonts->AddFontFromFileTTF(XorStrA("C:\\Windows\\Fonts\\Ruda-Bold.ttf"), 18);
}

#pragma endregion