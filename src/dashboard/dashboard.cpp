
#include "util/pch.h"

#include <imgui/imgui.h>

#include "events/event.h"
#include "events/application_event.h"
#include "events/mouse_event.h"
#include "events/key_event.h"
#include "util/io/serializer_data.h"
#include "util/io/serializer_yaml.h"
#include "util/ui/panel_collection.h"
#include "util/math/constance.h"
#include "application.h"
#include "config/imgui_config.h"

#include "dashboard.h"


namespace AT {

    dashboard::dashboard() {}
    
    
    dashboard::~dashboard() {}



    bool dashboard::init() {

        PROFILE_APPLICATION_FUNCTION();

        // =========== Demonstrate a long startup process (just replace with custom logic) ===========
        bool long_startup_process = false;
		AT::serializer::yaml(config::get_filepath_from_configtype(util::get_executable_path(), config::file::app_settings), "general_settings", AT::serializer::option::load_from_file)
			.entry(KEY_VALUE(long_startup_process));

        if (long_startup_process)
            std::this_thread::sleep_for(std::chrono::milliseconds(2500));  // 2.5s
        // ===========================================================================================

        LOG_INIT
        return true;
    }


    bool dashboard::shutdown() {

        PROFILE_APPLICATION_FUNCTION();
        LOG_SHUTDOWN
        return true;
    }


    void dashboard::on_crash() {
        
        LOG(Error, "Crash occurred")
    }


    void dashboard::update(f32 delta_time) {

        PROFILE_APPLICATION_FUNCTION();
    }


    void dashboard::draw(f32 delta_time) {
        
        PROFILE_APPLICATION_FUNCTION();
        
        // create a full-window dockspace
        {
            auto viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
            ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y));
            ImGui::SetNextWindowViewport(viewport->ID);

            ImGuiWindowFlags host_window_flags = 0;
            host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
            host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("main_content_area", NULL, host_window_flags);
                ImGui::PopStyleVar(3);

                static ImGuiDockNodeFlags dockspace_flags = 0;
                ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            ImGui::End();
        }

        // main content
        ImGui::ShowDemoWindow();
        ImPlot::ShowDemoWindow();

    }


    void dashboard::on_event(event& event) {}
    
    
    void dashboard::draw_init_UI(f32 delta_time) {

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        // Set window to cover the entire viewport
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | 
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | 
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        
        ImGui::Begin("Initialization", nullptr, window_flags);
        {
            // Center content in the window
            ImGui::SetCursorPos(ImVec2((viewport->WorkSize.x - 200) * 0.5f, (viewport->WorkSize.y - 100) * 0.5f));

            ImGui::PushFont(application::get().get_imgui_config_ref()->get_font(UI::font_type::giant));
            ImGui::Text("Initializing...");
            ImGui::PopFont();

            // Center the loading indicator
            ImGui::SetCursorPosX((viewport->WorkSize.x - 60) * 0.5f);
            UI::loading_indicator_circle("##loading_indicator", 30, 13, 5);
        }
        ImGui::End();
        
        ImGui::PopStyleVar(2);
    }

}
