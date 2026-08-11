
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

        m_audio_devices = audio::recorder::enumerate_devices();
        if (!m_audio_devices.empty()) {
            m_selected_device_index = 0; // default to first
        }

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


        // Audio Recorder Panel
        if (m_show_audio_panel) {
            ImGui::Begin("Audio Recorder", &m_show_audio_panel);

            // Device selection dropdown
            if (!m_audio_devices.empty()) {
                const char* preview = m_audio_devices[m_selected_device_index].description.c_str();
                if (ImGui::BeginCombo("Output Device", preview)) {
                    for (int i = 0; i < m_audio_devices.size(); ++i) {
                        bool isSelected = (m_selected_device_index == i);
                        if (ImGui::Selectable(m_audio_devices[i].description.c_str(), isSelected)) {
                            m_selected_device_index = i;
                        }
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            } else {
                ImGui::Text("No output devices found.");
            }

            // Filename input
            char filename_buf[256];
            strncpy(filename_buf, m_output_filename.c_str(), sizeof(filename_buf));
            filename_buf[sizeof(filename_buf)-1] = '\0';
            if (ImGui::InputText("Filename", filename_buf, sizeof(filename_buf))) {
                m_output_filename = filename_buf;
            }

            // Recording status and controls
            bool is_recording = m_audio_recorder.is_recording();
            if (is_recording) {
                auto elapsed = m_audio_recorder.elapsed_time();
                uint64_t bytes = m_audio_recorder.bytes_recorded();
                ImGui::Text("Recording...  Time: %lld s  Size: %.2f KB",
                            (long long)elapsed.count(), bytes / 1024.0);
                if (ImGui::Button("Stop Recording")) {
                    m_audio_recorder.stop_recording();
                }
            } else {
                if (ImGui::Button("Start Recording")) {
                    if (m_selected_device_index >= 0 && m_selected_device_index < m_audio_devices.size()) {
                        m_audio_recorder.start_recording(m_audio_devices[m_selected_device_index].name, m_output_filename);
                    }
                }
            }

            ImGui::End();
        }
        
        // // main content
        // ImGui::ShowDemoWindow();
        // ImPlot::ShowDemoWindow();

    }


    void dashboard::on_event(event& event) {}
    
    
    void dashboard::draw_init_UI(f32 delta_time) {

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        ImGui::Begin("Initialization", nullptr, window_flags);
        {

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
        }
        ImGui::End();
    }

}
