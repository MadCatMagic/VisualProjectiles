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
    
    AddCanvas();
    AddCanvas();
    RegisterJSONCommands();
}

void App::Update()
{
}

void App::UI(struct ImGuiIO* io)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (ImGui::MenuItem("Console", NULL, Console::instance->enabled))
                Console::instance->enabled = !Console::instance->enabled;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
	ImGui::DockSpaceOverViewport();
	
	ImGui::Begin("App");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io->Framerate, io->Framerate);
    ImGui::Separator();

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

    if (ImGui::Button("New Sim"))
        ImGui::OpenPopup("sim_add_popup");

    if (ImGui::BeginPopupContextItem("sim_add_popup"))
    {
        for (const std::string& name : GetSimulationFactory().Names())
            if (ImGui::Selectable(name.c_str()))
            {
                sims.push_back(GetSimulationFactory().Build(name));
                simTabOpen.push_back(true);
            }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
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
        canvases[i]->CreateWindow(i, disableControls);

    // recalculate curves
    for (Simulation* sim : sims)
        if (sim->enabled)
            sim->Calculate();

    for (ControlNode* node : ControlNode::aliveNodes)
        node->changedThisFrame = false;

    for (int i = 0; i < (int)canvases.size(); i++)
        canvases[i]->CreateSims(sims, t, disableControls);

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

void App::AddCanvas()
{
    Canvas* canvas = new Canvas();
    canvas->InitCanvas();
    canvases.push_back(canvas);
}
