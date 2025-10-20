
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
#include "visual_programming/visual_programming_editor.h"
#include "visual_programming/simplified_graph_editor.h"


#include "dashboard.h"


namespace AT {

    
    dashboard::dashboard()
        : m_visual_editor(std::make_unique<visual_programming_editor>())
        , m_simple_visual_editor(std::make_unique<simplified_graph_editor>())  { 

    }
    
    
    dashboard::~dashboard() { }



    bool dashboard::init() {

        PROFILE_APPLICATION_FUNCTION();

        // =========== Demonstrate a long startup process (just replace with custom logic) ===========
        // bool long_startup_process = false;
		// AT::serializer::yaml(config::get_filepath_from_configtype(util::get_executable_path(), config::file::app_settings), "general_settings", AT::serializer::option::load_from_file)
		// 	.entry(KEY_VALUE(long_startup_process));

        // if (long_startup_process)
        //     std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // 1s
        // ===========================================================================================
        
        VALIDATE(m_visual_editor->initialize(), return false, "", "Failed to initialize the visual programming editor")
        VALIDATE(m_simple_visual_editor->initialize(), return false, "", "Failed to initialize the visual programming editor")
        
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
        
        m_visual_editor->update(delta_time);
        m_simple_visual_editor->update(delta_time);
    }


    void dashboard::draw(const f32 delta_time) {

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

        
        // Add menu bar with save/load options
        // if (ImGui::BeginMainMenuBar()) {
        //     if (ImGui::BeginMenu("File")) {
        //         if (ImGui::MenuItem("New")) {
        //             if (m_unsavedChanges) {
        //                 // TODO: Add confirmation dialog
        //             }
        //             m_editor = ImFlow::ImNodeFlow{}; // Reset editor
        //             m_firstTime = true;
        //             m_unsavedChanges = false;
        //             m_currentFile.clear();
        //         }
        //
        //         if (ImGui::MenuItem("Open", "Ctrl+O")) {
        //             // TODO: Implement file dialog      For now, use a fixed path
        //             std::filesystem::path open_path = util::get_executable_path() / "visual_programming_test" / "saved_graph.yaml";
        //             VALIDATE(std::filesystem::exists(open_path), , "Loading file [" << open_path.string().c_str() << "]", "File [" << open_path.string().c_str() << "] does not exist");
        //             if (std::filesystem::exists(open_path)) {
        //
        //                 // Node factory function type
        //                 std::unordered_map<std::string, ImFlow::NodeFactory> node_factories = {
        //                     {"BeginNode",           [this](const ImVec2& pos) { return m_editor.addNode<BeginNode>(pos); }},
        //                     {"AddNode",             [this](const ImVec2& pos) { return m_editor.addNode<AddNode>(pos); }},
        //                     {"MultiplyNode",        [this](const ImVec2& pos) { return m_editor.addNode<MultiplyNode>(pos); }},
        //                     {"SubtractNode",        [this](const ImVec2& pos) { return m_editor.addNode<SubtractNode>(pos); }},
        //                     {"MultiOperationNode",  [this](const ImVec2& pos) { return m_editor.addNode<MultiOperationNode>(pos); }},
        //                     {"PlotterNode",         [this](const ImVec2& pos) { return m_editor.addNode<PlotterNode>(pos); }},
        //                     {"comment_node",         [this](const ImVec2& pos) { return m_editor.addNode<comment_node>(pos); }}
        //                 };
        //                 m_editor.load(open_path, node_factories);
        //                 m_currentFile = open_path;
        //                 m_unsavedChanges = false;
        //
        //                 // FIX: Ensure comment nodes update their bounds after loading
        //                 auto& nodes = m_editor.getNodes();
        //                 for (auto& [id, node] : nodes) {
        //                     if (auto comment_node = std::dynamic_pointer_cast<comment_node>(node)) {
        //                         comment_node->updateCommentBounds();
        //                     }
        //                 }
        //             }
        //         }
        //
        //         if (ImGui::MenuItem("Save", "Ctrl+S", false, !m_currentFile.empty())) {
        //             m_editor.save(m_currentFile);
        //             m_unsavedChanges = false;
        //         }
        //
        //         if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
        //             // TODO: Implement file dialog      For now, use a fixed path
        //             std::filesystem::path save_path = util::get_executable_path() / "visual_programming_test" / "saved_graph.yaml";
        //             m_editor.save(save_path);
        //             m_currentFile = save_path;
        //             m_unsavedChanges = false;
        //         }
        //
        //         ImGui::Separator();
        //
        //         if (ImGui::MenuItem("Quit", "Alt+F4")) {
        //             application::get().close_application();
        //         }
        //
        //         ImGui::EndMenu();
        //     }
        //
        //     if (ImGui::BeginMenu("Edit")) {
        //         if (ImGui::MenuItem("Select All", "Ctrl+A")) {
        //             // Select all nodes
        //             auto& nodes = m_editor.getNodes();
        //             for (auto& [id, node] : nodes) {
        //                 node->selected(true);
        //             }
        //         }
        //
        //         if (ImGui::MenuItem("Delete Selected", "Del")) {
        //             // Delete selected nodes
        //             auto& nodes = m_editor.getNodes();
        //             for (auto it = nodes.begin(); it != nodes.end(); ) {
        //                 if (it->second->isSelected()) {
        //                     it = nodes.erase(it);
        //                     m_unsavedChanges = true;
        //                 } else {
        //                     ++it;
        //                 }
        //             }
        //         }
        //
        //         ImGui::EndMenu();
        //     }
        //
        //     // Show current file and unsaved changes indicator
        //     ImGui::SameLine(ImGui::GetWindowWidth() - 200);
        //     std::string status = m_currentFile.empty() ? "Untitled" : m_currentFile.filename().string();
        //     if (m_unsavedChanges) status += " *";
        //     ImGui::Text("%s", status.c_str());
        //
        //     ImGui::EndMainMenuBar();
        // }
        

        ImGui::Begin("Node Editor");
        {
            ImVec2 available_size = ImGui::GetContentRegionAvail();
            m_visual_editor->set_size(available_size);
            m_visual_editor->draw(delta_time);
        }
        ImGui::End();
        ImGui::Begin("Simple Node Editor");
        {
            ImVec2 available_size = ImGui::GetContentRegionAvail();
            m_simple_visual_editor->set_size(available_size);
            m_simple_visual_editor->draw(delta_time);
        }
        ImGui::End();

        ImGui::Begin("Settings");
        {
            m_visual_editor->draw_settings_panel();
        }
        ImGui::End();
    }

    void dashboard::on_event(event& event) {

        bool handled = false;
        event_dispatcher dispatcher(event);
        handled = dispatcher.dispatch<key_event>([this](key_event& e) { 
            return m_visual_editor->on_key_event(e) || m_simple_visual_editor->on_key_event(e); 
        });
        
    }
    
    
    void dashboard::draw_init_UI(f32 delta_time) {

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        ImGui::Begin("Initialization", nullptr, window_flags);
        {

            ImGui::PushFont(application::get().get_imgui_config_ref()->get_font(UI::font_type::giant));
            const char* text = "Initializing...";
            const float target_font_size = 50.0f;
            ImVec2 base_text_size = ImGui::CalcTextSize(text);                                      // Calculate base text size at default font scale
            float scale = (base_text_size.y > 0) ? target_font_size / base_text_size.y : 1.0f;      // Calculate required scale to reach target font size
            ImVec2 available = ImGui::GetContentRegionAvail() * 0.9f;                               // Get available space with 10% margin
            ImVec2 scaled_size = base_text_size * scale;                                            // Calculate scaled text size
            
            if (scaled_size.x > available.x || scaled_size.y > available.y) {                       // Adjust scale if needed to fit available space
                float width_ratio = available.x / scaled_size.x;
                float height_ratio = available.y / scaled_size.y;
                scale *= (width_ratio < height_ratio) ? width_ratio : height_ratio;
            }
            
            // Set font scale and calculate final position
            ImGui::SetWindowFontScale(scale);
            ImVec2 text_size = ImGui::CalcTextSize(text);
            ImVec2 position = (ImGui::GetContentRegionAvail() - text_size) * 0.5f;
            
            ImGui::SetCursorPos(position);
            ImGui::TextUnformatted(text);
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopFont();

            UI::shift_cursor_pos((ImGui::GetContentRegionAvail().x / 2) - 30, 30);
            UI::loading_indicator_circle("##loading_indicator", 30, 13, 5);

        }
        ImGui::End();
    }

}
