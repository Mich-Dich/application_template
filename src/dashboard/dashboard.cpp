
#include "util/pch.h"

#include <imgui/imgui.h>
#include <ImNodeFlow.h>

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
#include "visual_programming/nodes.h"

#include "dashboard.h"


namespace AT {


    ImFlow::ImNodeFlow m_editor{};  // Add this member variable
    bool m_firstTime = true;      // Add this to track first draw
    
    dashboard::dashboard() { 

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

        if (m_firstTime) {
            // Setup background first
            auto& style = m_editor.getStyle();
            setupCustomBackground(m_editor);
            
            // Setup right-click context menu for adding nodes with search and tree view
            m_editor.rightClickPopUpContent([this](ImFlow::BaseNode* node) {
                if (node == nullptr) {
                    
                    // Search bar
                    static char search_buffer[128] = "";
                    ImGui::SetNextItemWidth(-1);
                    if (ImGui::InputTextWithHint("##Search", "Search nodes...", search_buffer, IM_ARRAYSIZE(search_buffer))) {
                        // Search filter is applied in the drawing logic below
                    }
                    
                    ImGui::Separator();
                    
                    // Define all available nodes with categories
                    struct NodeDefinition {
                        const char* name;
                        const char* category;
                        std::function<void()> creator;
                        const char* description;
                    };
                    
                    static std::vector<NodeDefinition> nodes = {
                        {"Begin", "Execution", [this]() {
                            m_editor.placeNode<BeginNode>()->setTitle("Begin");
                        }, "Start execution flow"},
                        
                        {"Add", "Math Operations", [this]() {
                            m_editor.placeNode<AddNode>()->setTitle("Add Numbers");
                        }, "A + B"},
                        
                        {"Multiply", "Math Operations", [this]() {
                            m_editor.placeNode<MultiplyNode>()->setTitle("Multiply Numbers");
                        }, "A × B"},
                        
                        {"Subtract", "Math Operations", [this]() {
                            m_editor.placeNode<SubtractNode>()->setTitle("Subtract Numbers");
                        }, "A - B"},
                        
                        {"Multi Operation", "Math Operations", [this]() {
                            m_editor.placeNode<MultiOperationNode>()->setTitle("Multi Operation");
                        }, "16 math operations in one node"},
                        
                        {"Plotter", "Visualization", [this]() {
                            m_editor.placeNode<PlotterNode>()->setTitle("Data Plotter");
                        }, "Create various types of plots and charts"},

                        {"Comment", "Organization", [this]() {
                            m_editor.placeNode<CommentNode>()->setTitle("Comment");
                        }, "Group nodes with a comment box"},
                    };
                    
                    // Group nodes by category
                    std::map<std::string, std::vector<NodeDefinition>> categorized_nodes;
                    for (const auto& node_def : nodes) {
                        categorized_nodes[node_def.category].push_back(node_def);
                    }
                    
                    // Filter nodes based on search
                    std::string search_lower = search_buffer;
                    std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);
                    
                    bool any_visible = false;
                    
                    ImGui::BeginChild("NodeList", ImVec2(300, 350), true);
                    for (const auto& [category, node_list] : categorized_nodes) {
                        std::vector<NodeDefinition> filtered_nodes;
                        
                        // Filter nodes in this category
                        for (const auto& node_def : node_list) {
                            std::string name_lower = node_def.name;
                            std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
                            std::string desc_lower = node_def.description;
                            std::transform(desc_lower.begin(), desc_lower.end(), desc_lower.begin(), ::tolower);
                            std::string category_lower = category;
                            std::transform(category_lower.begin(), category_lower.end(), category_lower.begin(), ::tolower);
                            
                            if (search_lower.empty() || 
                                name_lower.find(search_lower) != std::string::npos ||
                                desc_lower.find(search_lower) != std::string::npos ||
                                category_lower.find(search_lower) != std::string::npos) {
                                filtered_nodes.push_back(node_def);
                            }
                        }
                        
                        if (!filtered_nodes.empty()) {
                            any_visible = true;
                            
                            // Category tree node (always expanded by default)
                            ImGuiTreeNodeFlags category_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed;
                            if (ImGui::TreeNodeEx(category.c_str(), category_flags)) {
                                for (const auto& node_def : filtered_nodes) {
                                    ImGui::PushID(node_def.name);
                                    
                                    // Node button with description
                                    if (ImGui::Selectable(node_def.name, false, ImGuiSelectableFlags_AllowDoubleClick)) {
                                        if (ImGui::IsMouseDoubleClicked(0)) {
                                            node_def.creator();
                                            ImGui::CloseCurrentPopup();
                                            memset(search_buffer, 0, sizeof(search_buffer)); // Clear search
                                        }
                                    }
                                    
                                    // Tooltip with description
                                    if (ImGui::IsItemHovered()) {
                                        ImGui::BeginTooltip();
                                        ImGui::TextUnformatted(node_def.description);
                                        ImGui::EndTooltip();
                                        
                                        // Single click also works for creation
                                        if (ImGui::IsMouseClicked(0)) {
                                            node_def.creator();
                                            ImGui::CloseCurrentPopup();
                                            memset(search_buffer, 0, sizeof(search_buffer)); // Clear search
                                        }
                                    }
                                    
                                    ImGui::PopID();
                                }
                                ImGui::TreePop();
                            }
                        }
                    }
                    
                    // No results message
                    if (!any_visible && !search_lower.empty()) {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No nodes found matching: '%s'", search_buffer);
                        ImGui::Text("Try different search terms");
                    }
                    
                    ImGui::EndChild();
                    
                    // Bottom button bar
                    ImGui::Separator();
                    if (ImGui::Button("Clear Search", ImVec2(100, 0))) {
                        memset(search_buffer, 0, sizeof(search_buffer));
                    }
                    ImGui::SameLine();
                    ImGui::TextDisabled("%zu nodes available", nodes.size());
                    
                } else {
                    // Right-click on a specific node - show simple context menu
                    ImGui::SetWindowSize(ImVec2(200, 300)); // Auto-size for node menu
                    
                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Node: %s", node->getName().c_str());
                    ImGui::Separator();
                    
                    if (ImGui::MenuItem("Delete")) {
                        node->destroy();
                    }
                    
                    if (ImGui::MenuItem("Duplicate")) {
                        // Basic duplication logic
                        auto pos = node->getPos();
                        auto new_pos = ImVec2(pos.x + 50, pos.y + 50);
                        
                        // Handle different node types (simplified - you might want to copy properties)
                        if (dynamic_cast<BeginNode*>(node)) {
                            m_editor.addNode<BeginNode>(new_pos)->setTitle(node->getName() + " Copy");
                        } else if (dynamic_cast<AddNode*>(node)) {
                            m_editor.addNode<AddNode>(new_pos)->setTitle(node->getName() + " Copy");
                        } else if (dynamic_cast<MultiplyNode*>(node)) {
                            m_editor.addNode<MultiplyNode>(new_pos)->setTitle(node->getName() + " Copy");
                        } else if (dynamic_cast<SubtractNode*>(node)) {
                            m_editor.addNode<SubtractNode>(new_pos)->setTitle(node->getName() + " Copy");
                        } else if (dynamic_cast<MultiOperationNode*>(node)) {
                            m_editor.addNode<MultiOperationNode>(new_pos)->setTitle(node->getName() + " Copy");
                        } else if (dynamic_cast<PlotterNode*>(node)) {
                            m_editor.addNode<PlotterNode>(new_pos)->setTitle(node->getName() + " Copy");
                        }
                    }
                    
                    if (ImGui::MenuItem("Rename")) {
                        // Simple rename - you could implement a proper dialog
                        std::string new_name = node->getName() + " Renamed";
                        node->setTitle(new_name);
                    }

                    ImGui::SeparatorText("Comment Assignment");

                    // Find all comment nodes in the editor
                    auto& allNodes = m_editor.getNodes();
                    std::vector<std::shared_ptr<CommentNode>> commentNodes;
                    std::vector<std::shared_ptr<CommentNode>> containingComments;

                    for (auto& [id, nodePtr] : allNodes) {
                        if (auto commentNode = std::dynamic_pointer_cast<CommentNode>(nodePtr)) {
                            commentNodes.push_back(commentNode);
                            // Check if this node is already in the comment
                            if (commentNode->getContainedNodes().count(node->getUID()) > 0) {
                                containingComments.push_back(commentNode);
                            }
                        }
                    }

                    if (!commentNodes.empty()) {
                        // Show which comments already contain this node
                        if (!containingComments.empty()) {
                            ImGui::TextDisabled("Currently in:");
                            for (auto& commentNode : containingComments) {
                                if (ImGui::MenuItem(("Remove from: " + commentNode->getCommentText()).c_str())) {
                                    commentNode->removeContainedNode(node->getUID());
                                }
                            }
                            ImGui::Separator();
                        }
                        
                        ImGui::TextDisabled("Add to comment:");
                        for (auto& commentNode : commentNodes) {
                            // Don't show comments that already contain this node
                            if (commentNode->getContainedNodes().count(node->getUID()) == 0) {
                                std::string menuText = commentNode->getCommentText();
                                if (menuText.length() > 30) {
                                    menuText = menuText.substr(0, 27) + "...";
                                }
                                if (ImGui::MenuItem(menuText.c_str())) {
                                    commentNode->addContainedNode(node->getUID());
                                }
                            }
                        }
                    } else {
                        ImGui::TextDisabled("No comment nodes available");
                    }
                }
            });
                
            // Now create initial node
            auto begin_node = m_editor.addNode<BeginNode>({110, 100});
            begin_node->setTitle("Begin");

            m_firstTime = false;
        }

        ImGui::Begin("Node Editor");
        {
            ImVec2 available_size = ImGui::GetContentRegionAvail();
            m_editor.setSize(available_size);
            m_editor.update();
        }
        ImGui::End();

        ImGui::Begin("Settings");
        {
            // Initialize static variables with current colors
            static glm::vec4 s_background_color{33.0f/255.0f, 33.0f/255.0f, 33.0f/255.0f, 1.0f};
            static glm::vec4 s_grid_color{75.0f/255.0f, 75.0f/255.0f, 75.0f/255.0f, 80.0f/255.0f};
            static glm::vec4 s_subGrid_color{50.0f/255.0f, 50.0f/255.0f, 50.0f/255.0f, 40.0f/255.0f};

            UI::begin_table("settings", false);
            static bool color_updated = true;
            color_updated |= UI::table_row_slider_color("Background", s_background_color);
            color_updated |= UI::table_row_slider_color("Grid", s_grid_color);
            color_updated |= UI::table_row_slider_color("Subgrid", s_subGrid_color);
            UI::end_table();

            if (color_updated) {
                #define COLOR_TO_IM_INT32(var) IM_COL32(var.x * 255, var.y * 255, var.z * 255, var.w * 255)
                
                auto& style = m_editor.getStyle();
                style.colors.background = COLOR_TO_IM_INT32(s_background_color);
                style.colors.grid = COLOR_TO_IM_INT32(s_grid_color);
                style.colors.subGrid = COLOR_TO_IM_INT32(s_subGrid_color);
                
                // Also update the context background color
                m_editor.getGrid().config().color = style.colors.background;

                #undef COLOR_TO_IM_INT32
                color_updated = false;
            }
        }
        ImGui::End();
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
