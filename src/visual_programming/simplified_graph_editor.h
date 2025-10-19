#pragma once

#include <ImNodeFlow.h>
#include "simplified_nodes.h"

namespace AT {

    class simplified_graph_editor {
    public:

        simplified_graph_editor();
        ~simplified_graph_editor() = default;

        bool initialize();

        void update(const f32 delta_time);
        void draw(const f32 delta_time);
        void set_size(const ImVec2 new_size);

        void update_grid_color(glm::vec4 background_color, glm::vec4 grid_color, glm::vec4 subgrid_color);

        // File operations
        void new_graph();
        void load(const std::filesystem::path& filename);
        void save(const std::filesystem::path& filename);
        void save_as(const std::filesystem::path& filename);

        // Getters for dashboard to check state
        bool has_unsaved_changes() const { return m_unsavedChanges; }
        const std::filesystem::path& get_current_file() const { return m_currentFile; }
        void set_unsaved_changes(bool state) { m_unsavedChanges = state; }

        // Menu bar operations
        void draw_menu_bar();
        void draw_settings_panel();

        bool on_key_event(key_event& event);

    private:

        void setup_node_factories();
        void setup_right_click_menu();
        void create_default_pc_node();

        ImFlow::ImNodeFlow m_editor{};
        std::filesystem::path m_currentFile;
        bool m_unsavedChanges = false;
        bool m_initialized = false;
        char m_searchBuffer[128] = "";
        bool m_searchActive = false;
        std::unordered_map<std::string, ImFlow::NodeFactory> m_node_factories;
    };
}
