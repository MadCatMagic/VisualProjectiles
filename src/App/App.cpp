#include "App/App.h"

#include "imgui.h"
#include "Engine/Console.h"
#include "App/JSON.h"

void App::Initialize()
{
    c.GenerateAllTextLODs();
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


	ImGui::End();

    c.CreateWindow();
}

void App::Release()
{
}
