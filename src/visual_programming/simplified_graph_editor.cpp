#include "util/pch.h"
#include "util/io/serializer_yaml.h"
#include "util/ui/panel_collection.h"
#include "events/key_event.h"
#include "simplified_graph_editor.h"

namespace AT {

    simplified_graph_editor::simplified_graph_editor() {
        setup_node_factories();
        memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
    }

    bool simplified_graph_editor::initialize() {
        
        // Setup background first
        auto& style = m_editor.getStyle();
        style.grid_size = 80.0f;
        style.grid_subdivisions = 8.0f;

        // Dark theme for simplified editor
        const glm::vec4 s_background_color  {0.1f, 0.1f, 0.1f, 1.0f};
        const glm::vec4 s_grid_color        {0.3f, 0.3f, 0.3f, 0.3f};
        const glm::vec4 s_sub_grid_color    {0.2f, 0.2f, 0.2f, 0.1f};
        update_grid_color(s_background_color, s_grid_color, s_sub_grid_color);

        setup_right_click_menu();
        create_default_pc_node();

        m_initialized = true;
        return true;
    }

    void simplified_graph_editor::create_default_pc_node() {
        // Create PC node at a fixed position
        auto pc_node = m_editor.addNode<PCNode>({200, 150});
        pc_node->setTitle("Main PC");
    }

    void simplified_graph_editor::setup_node_factories() {
        m_node_factories["PCNode"] = [this](const ImVec2& pos) { 
            return m_editor.addNode<PCNode>(pos);
        };
        m_node_factories["DeviceNode"] = [this](const ImVec2& pos) { 
            return m_editor.addNode<DeviceNode>(pos);
        };
    }

    void simplified_graph_editor::setup_right_click_menu() {
        m_editor.rightClickPopUpContent([this](ImFlow::BaseNode* node) {
            
            ImVec2 contextMenuPos = ImGui::GetWindowPos();
            if (node == nullptr) {
                // Right-click on empty space - show simplified node menu
                
                // Search bar
                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputTextWithHint("##Search", "Search devices...", m_searchBuffer, IM_ARRAYSIZE(m_searchBuffer), 
                    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                    m_searchActive = true;
                }

                // Auto-focus on the search bar when popup opens
                static bool focusSearch = true;
                if (focusSearch) {
                    ImGui::SetKeyboardFocusHere(-1);
                    focusSearch = false;
                }

                // Reset focus flag when popup closes
                if (!ImGui::IsPopupOpen("RightClickPopUp")) {
                    focusSearch = true;
                }
                
                ImGui::Separator();

                // Filter nodes based on search
                std::string search_lower = m_searchBuffer;
                std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);
                
                ImGui::BeginChild("NodeList", ImVec2(250, 200), true);
                
                bool any_visible = false;
                for (const auto& node_def : simplified_node_list) {
                    std::string name_lower = node_def.name;
                    std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
                    std::string desc_lower = node_def.description;
                    std::transform(desc_lower.begin(), desc_lower.end(), desc_lower.begin(), ::tolower);
                    
                    if (search_lower.empty() || 
                        name_lower.find(search_lower) != std::string::npos ||
                        desc_lower.find(search_lower) != std::string::npos) {
                        
                        any_visible = true;
                        ImGui::PushID(node_def.name);
                        
                        ImGui::Selectable(node_def.name, false);
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::TextUnformatted(node_def.description);
                            ImGui::EndTooltip();
                            
                            if (ImGui::IsMouseClicked(0)) {
                                ImVec2 gridPos = m_editor.screen2grid(contextMenuPos);
                                node_def.creator(m_editor, ImVec2(gridPos.x + 50, gridPos.y + 50));
                                ImGui::CloseCurrentPopup();
                                memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
                                m_unsavedChanges = true;
                                m_searchActive = false;
                            }
                        }
                        
                        ImGui::PopID();
                    }
                }
                
                if (!any_visible && !search_lower.empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No devices found matching: '%s'", m_searchBuffer);
                }
                
                ImGui::EndChild();
                
                ImGui::Separator();
                if (ImGui::Button("Clear Search", ImVec2(100, 0))) {
                    memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
                    m_searchActive = false;
                }
                
            } else {
                // Right-click on a specific node
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Node: %s", node->getName().c_str());
                ImGui::Separator();
                
                // Only allow deletion for Device nodes, not PC node
                if (dynamic_cast<PCNode*>(node) == nullptr) {
                    if (ImGui::MenuItem("Delete")) {
                        node->destroy();
                        m_unsavedChanges = true;
                    }
                } else {
                    ImGui::TextDisabled("PC node cannot be deleted");
                }
            }
        });
    }

    void simplified_graph_editor::update(const f32 delta_time) {
        (void)delta_time; // Prevent unused parameter warning
    }

    void simplified_graph_editor::draw(const f32 delta_time) {
        
        if (!m_initialized) return;
            
        try {
            m_editor.update();
        } catch (const std::exception& e) {
            LOG(Error, "Error in simplified editor update: " << e.what())
            m_initialized = false;
            initialize();
        }
    }

    void simplified_graph_editor::set_size(const ImVec2 new_size) { 
        m_editor.setSize(new_size); 
    }

    void simplified_graph_editor::update_grid_color(glm::vec4 background_color, glm::vec4 grid_color, glm::vec4 subgrid_color) {

        #define COLOR_TO_IM_INT32(var) IM_COL32(var.x * 255, var.y * 255, var.z * 255, var.w * 255)
    
        auto& style = m_editor.getStyle();
        style.colors.background = COLOR_TO_IM_INT32(background_color);
        style.colors.grid = COLOR_TO_IM_INT32(grid_color);
        style.colors.subGrid = COLOR_TO_IM_INT32(subgrid_color);
    
        // Also update the context background color
        m_editor.getGrid().config().color = style.colors.background;
        #undef COLOR_TO_IM_INT32
    }

    void simplified_graph_editor::new_graph() {
        if (m_unsavedChanges) {
            // TODO: Add confirmation dialog
        }

        // Clear existing nodes and links
        auto& nodes = m_editor.getNodes();
        nodes.clear();
        
        auto links = m_editor.getLinks();
        for (auto& weak_link : links) {
            if (auto link = weak_link.lock()) {
                // Links will be automatically cleaned up
            }
        }
        
        // Re-initialize with default setup
        initialize();
        m_unsavedChanges = false;
        m_currentFile.clear();
    }

    void simplified_graph_editor::load(const std::filesystem::path& filename) {
        if (!std::filesystem::exists(filename)) {
            return;
        }

        m_editor.load(filename, m_node_factories);
        m_currentFile = filename;
        m_unsavedChanges = false;
    }

    void simplified_graph_editor::save(const std::filesystem::path& filename) {
        m_editor.save(filename);
        m_currentFile = filename;
        m_unsavedChanges = false;
    }

    void simplified_graph_editor::save_as(const std::filesystem::path& filename) {
        save(filename);
    }

    void simplified_graph_editor::draw_menu_bar() {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                new_graph();
            }
            
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // TODO: Implement file dialog
                std::filesystem::path open_path = util::get_executable_path() / "simplified_graph" / "saved_graph.yaml";
                if (std::filesystem::exists(open_path)) {
                    load(open_path);
                }
            }
            
            if (ImGui::MenuItem("Save", "Ctrl+S", false, !m_currentFile.empty())) {
                save(m_currentFile);
            }
            
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                // TODO: Implement file dialog
                std::filesystem::path save_path = util::get_executable_path() / "simplified_graph" / "saved_graph.yaml";
                save_as(save_path);
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Quit", "Alt+F4")) {
                // This would be handled by the dashboard
            }
            
            ImGui::EndMenu();
        }
        
        // Show current file and unsaved changes indicator
        ImGui::SameLine(ImGui::GetWindowWidth() - 200);
        std::string status = m_currentFile.empty() ? "Untitled" : m_currentFile.filename().string();
        if (m_unsavedChanges) status += " *";
        ImGui::Text("%s", status.c_str());
    }

    void simplified_graph_editor::draw_settings_panel() {
        
        // Initialize static variables with current colors
        static glm::vec4 s_background_color  {0.1f, 0.1f, 0.1f, 1.0f};
        static glm::vec4 s_grid_color        {0.3f, 0.3f, 0.3f, 0.3f};
        static glm::vec4 s_sub_grid_color    {0.2f, 0.2f, 0.2f, 0.1f};

        UI::begin_table("settings", false);
        static bool color_updated = false;
        color_updated |= UI::table_row_slider_color("Background", s_background_color);
        color_updated |= UI::table_row_slider_color("Grid", s_grid_color);
        color_updated |= UI::table_row_slider_color("Sub-grid", s_sub_grid_color);
        UI::end_table();

        if (color_updated) {
            update_grid_color(s_background_color, s_grid_color, s_sub_grid_color);
            color_updated = false;
        }
    }

    bool simplified_graph_editor::on_key_event(key_event& event) {
        // Handle key events if needed
        return false;
    }
}
