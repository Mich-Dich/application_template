
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

    void ShowExampleAppDockSpace(bool* p_open)
    {
        // Variables to configure the Dockspace example.
        static bool opt_fullscreen = true; // Is the Dockspace full-screen?
        static bool opt_padding = false; // Is there padding (a blank space) between the window edge and the Dockspace?
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None; // Config flags for the Dockspace

        // In this example, we're embedding the Dockspace into an invisible parent window to make it more configurable.
        // We set ImGuiWindowFlags_NoDocking to make sure the parent isn't dockable into because this is handled by the Dockspace.
        //
        // ImGuiWindowFlags_MenuBar is to show a menu bar with config options. This isn't necessary to the functionality of a
        // Dockspace, but it is here to provide a way to change the configuration flags interactively.
        // You can remove the MenuBar flag if you don't want it in your app, but also remember to remove the code which actually
        // renders the menu bar, found at the end of this function.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        // Is the example in Fullscreen mode?
        if (opt_fullscreen)
        {
            // If so, get the main viewport:
            const ImGuiViewport* viewport = ImGui::GetMainViewport();

            // Set the parent window's position, size, and viewport to match that of the main viewport. This is so the parent window
            // completely covers the main viewport, giving it a "full-screen" feel.
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);

            // Set the parent window's styles to match that of the main viewport:
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f); // No corner rounding on the window
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); // No border around the window

            // Manipulate the window flags to make it inaccessible to the user (no titlebar, resize/move, or navigation)
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            // The example is not in Fullscreen mode (the parent window can be dragged around and resized), disable the
            // ImGuiDockNodeFlags_PassthruCentralNode flag.
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so the parent window should not have its own background:
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // If the padding option is disabled, set the parent window's padding size to 0 to effectively hide said padding.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::Begin("DockSpace Demo", p_open, window_flags);

        // Remove the padding configuration - we pushed it, now we pop it:
        if (!opt_padding)
            ImGui::PopStyleVar();

        // Pop the two style rules set in Fullscreen mode - the corner rounding and the border size.
        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Check if Docking is enabled:
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            // If it is, draw the Dockspace with the DockSpace() function.
            // The GetID() function is to give a unique identifier to the Dockspace - here, it's "MyDockSpace".
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        else
        {
            // Docking is DISABLED - Show a warning message
            // ShowDockingDisabledMessage();
        }

        // This is to show the menu bar that will change the config settings at runtime.
        // If you copied this demo function into your own code and removed ImGuiWindowFlags_MenuBar at the top of the function,
        // you should remove the below if-statement as well.
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                // Display a menu item for each Dockspace flag, clicking on one will toggle its assigned flag.
                if (ImGui::MenuItem("Flag: NoSplit",                "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
                if (ImGui::MenuItem("Flag: NoResize",               "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0))                { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
                if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0))  { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
                if (ImGui::MenuItem("Flag: AutoHideTabBar",         "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0))          { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
                if (ImGui::MenuItem("Flag: PassthruCentralNode",    "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
                ImGui::Separator();

                // Display a menu item to close this example.
                if (ImGui::MenuItem("Close", NULL, false, p_open != NULL))
                    if (p_open != NULL) // Remove MSVC warning C6011 (NULL dereference) - the `p_open != NULL` in MenuItem() does prevent NULL derefs, but IntelliSense doesn't analyze that deep so we need to add this in ourselves.
                        *p_open = false; // Changing this variable to false will close the parent window, therefore closing the Dockspace as well.
                ImGui::EndMenu();
            }

            // Show a help marker that gives an overview of what this example is and does.
            // HelpMarker(
            //     "When docking is enabled, you can ALWAYS dock MOST window into another! Try it now!" "\n"
            //     "- Drag from window title bar or their tab to dock/undock." "\n"
            //     "- Drag from window menu button (upper-left button) to undock an entire node (all windows)." "\n"
            //     "- Hold SHIFT to disable docking." "\n"
            //     "This demo app has nothing to do with it!" "\n\n"
            //     "This demo app only demonstrates the use of ImGui::DockSpace() which allows you to manually create a docking node _within_ another window." "\n\n"
            //     "Read comments in ShowExampleAppDockSpace() for more details.");

            ImGui::EndMenuBar();
        }

        // End the parent window that contains the Dockspace:
        ImGui::End();
    }





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

        // Use the example dockspace implementation
        static bool dockspace_open = true; // Keep dockspace always open
        ShowExampleAppDockSpace(&dockspace_open);

        // Your existing windows - they will now dock into the example dockspace
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
