#include "App/App.h"

#include "imgui.h"
#include "Engine/Console.h"
#include "App/JSON.h"

#include "App/Simulation.h"
#include "App/SimulationFactory.h"

void App::Initialize()
{
    GetSimulationFactory().Register("Task One Projectile", SimulationBuilder<TaskOneProjectile>);

    //c.GenerateAllTextLODs();
    c.InitCanvas();
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
    ImGui::InputFloat2("g", &Simulation::gravity.x);

    ImGui::Separator();

    for (int n = 0; n < sims.size(); n++)
    {
        ImGui::PushID(n);

        // have to do it like this so ImGui doesn't freak the hell out
        bool drawTree = false;
        ImGui::SetNextItemOpen(simTabOpen[n]);
        // disgusting but necessary
        if (ImGui::TreeNodeEx((const void*)sims[n], ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick, sims[n]->name.c_str()))
            drawTree = true;

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

        if (drawTree)
        {
            static char buf[64] = "";
            strcpy_s(buf, sims[n]->name.c_str());

            if (ImGui::InputText("name", buf, 64))
                sims[n]->name = std::string(buf);

            ImGui::ColorEdit3("colour", &sims[n]->colour.x);

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

            ImGui::TreePop();
        }

        ImGui::PopID();
        ImGui::Separator();
    }

	ImGui::End();

    c.CreateWindow(sims);
}

void App::Release()
{
    for (Simulation* sim : sims)
        if (sim != nullptr)
            delete sim;
}
