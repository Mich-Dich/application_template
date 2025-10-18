#include "util/pch.h"

#include "util/io/serializer_yaml.h"
#include "util/ui/panel_collection.h"
#include "events/key_event.h"

#include "visual_programming_editor.h"


namespace AT {

    visual_programming_editor::visual_programming_editor() {
        
        // Somewhere in your initialization code
        // FunctionRegistry::getInstance().registerFunction({
        //     "Sine Wave",
        //     "Calculates sine of input angle in radians", 
        //     "Math",
        //     {{"angle", 0.0}},
        //     {{"result", 0.0}},
        //     [](const std::vector<FunctionValue>& inputs) -> std::vector<FunctionValue> {
        //         double angle = std::get<double>(inputs[0]);
        //         return {std::sin(angle)};
        //     }
        // });

        // FunctionRegistry::getInstance().registerFunction({
        //     "Power",
        //     "Raises base to exponent power",
        //     "Math", 
        //     {{"base", 0.0}, {"exponent", 0.0}},
        //     {{"result", 0.0}},
        //     [](const std::vector<FunctionValue>& inputs) -> std::vector<FunctionValue> {
        //         double base = std::get<double>(inputs[0]);
        //         double exponent = std::get<double>(inputs[1]);
        //         return {std::pow(base, exponent)};
        //     }
        // });

        setup_node_factories();
        memset(m_searchBuffer, 0, sizeof(m_searchBuffer)); // Initialize search buffer
    }


    bool visual_programming_editor::initialize() {
        
        // Setup background first
        auto& style = m_editor.getStyle();
        style.grid_size = 100.0f;
        style.grid_subdivisions = 10.0f;

        const glm::vec4 s_background_color  {1.f, 1.f, 1.f, 0.01f};
        const glm::vec4 s_grid_color        {1.f, 1.f, 1.f, 0.10f};
        const glm::vec4 s_sub_grid_color    {1.f, 1.f, 1.f, 0.05f};
        update_grid_color(s_background_color, s_grid_color, s_sub_grid_color);

        setup_right_click_menu();
            
        // Create initial node
        auto begin_node = m_editor.addNode<BeginNode>({110, 100});
        begin_node->setTitle("Begin");

        m_initialized = true;
        return true;
    }


    void visual_programming_editor::setup_node_factories() {
        m_node_factories = {
            {"BeginNode", [this](const ImVec2& pos) { return m_editor.addNode<BeginNode>(pos); }},
            {"math_expression_node", [this](const ImVec2& pos) { return m_editor.addNode<math_expression_node>(pos); }},
            {"PlotterNode", [this](const ImVec2& pos) { return m_editor.addNode<PlotterNode>(pos); }},
            {"comment_node", [this](const ImVec2& pos) { return m_editor.addNode<comment_node>(pos); }},
            {"FunctionNode", [this](const ImVec2& pos) { 
                // For serialization, need to handle this differently, MAYBE: store the function name and recreate it
                return m_editor.addNode<FunctionNode>(pos, FunctionRegistry::getInstance().getAllFunctions().begin()->second);
            }}
        };
    }


    void visual_programming_editor::setup_right_click_menu() {
        m_editor.rightClickPopUpContent([this](ImFlow::BaseNode* node) {
            
            ImVec2 contextMenuPos = ImGui::GetWindowPos();      // Store the position where the context menu was opened
            if (node == nullptr) {
                
                // Search bar - use member variable instead of static
                ImGui::SetNextItemWidth(-1);
                if (ImGui::InputTextWithHint("##Search", "Search nodes...", m_searchBuffer, IM_ARRAYSIZE(m_searchBuffer))) {
                    m_searchActive = true;
                }
                
                ImGui::Separator();

                auto completeNodeList = AT::getCompleteNodeList();          // Use the complete node list including functions
                
                // Group nodes by category
                std::map<std::string, std::vector<NodeDefinition>> categorized_nodes;
                for (const auto& node_def : node_list)
                    categorized_nodes[node_def.category].push_back(node_def);
                
                for (const auto& node_def : completeNodeList)
                    categorized_nodes[node_def.category].push_back(node_def);
                
                // Filter nodes based on search
                std::string search_lower = m_searchBuffer;
                std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);
                
                bool any_visible = false;
                
                ImGui::BeginChild("NodeList", ImVec2(300, 350), true);
                for (const auto& [category, nodes_in_category] : categorized_nodes) {
                    std::vector<NodeDefinition> filtered_nodes;
                    
                    // Filter nodes in this category
                    for (const auto& node_def : nodes_in_category) {
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
                        
                        ImGuiTreeNodeFlags category_flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed;
                        if (ImGui::TreeNodeEx(category.c_str(), category_flags)) {
                            for (const auto& node_def : filtered_nodes) {
                                ImGui::PushID(node_def.name);
                                
                                ImGui::Selectable(node_def.name, false);
                                if (ImGui::IsItemHovered()) {
                                    ImGui::BeginTooltip();
                                    ImGui::TextUnformatted(node_def.description);
                                    ImGui::EndTooltip();
                                    
                                    if (ImGui::IsMouseClicked(0)) {
                                        ImVec2 gridPos = m_editor.screen2grid(contextMenuPos);
                                        node_def.creator(m_editor, gridPos);
                                        ImGui::CloseCurrentPopup();
                                        memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
                                        m_unsavedChanges = true;
                                        m_searchActive = false;
                                    }
                                }
                                
                                ImGui::PopID();
                            }
                            ImGui::TreePop();
                        }
                    }
                }
                
                if (!any_visible && !search_lower.empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "No nodes found matching: '%s'", m_searchBuffer);
                    ImGui::Text("Try different search terms");
                }
                
                ImGui::EndChild();
                
                ImGui::Separator();
                if (ImGui::Button("Clear Search", ImVec2(100, 0))) {
                    memset(m_searchBuffer, 0, sizeof(m_searchBuffer));
                    m_searchActive = false;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("%zu nodes available", node_list.size());
                
            } else {        // Right-click on a specific node - show simple context menu

                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Node: %s", node->getName().c_str());
                ImGui::Separator();
                
                if (ImGui::MenuItem("Delete")) {
                    node->destroy();
                    m_unsavedChanges = true;
                }
                
                if (ImGui::MenuItem("Duplicate")) {
                    // Store position for duplication
                    contextMenuPos = ImGui::GetMousePos();
                    
                    // Basic duplication logic - place duplicate near the original with offset
                    auto pos = node->getPos();
                    auto new_pos = ImVec2(pos.x + 50, pos.y + 50);
                    
                    const auto& node_name = node->getName();

                    bool found_node_def = false;
                    for (const auto& def : node_list) {
                        if (def.name != node_name)
                            continue;

                        // Convert screen position to grid position for the duplicate
                        ImVec2 gridPos = m_editor.screen2grid(contextMenuPos);
                        // Use the context menu position with a small offset for the duplicate
                        def.creator(m_editor, ImVec2(gridPos.x + 50, gridPos.y + 50));
                        found_node_def = true;
                    }
                    VALIDATE(found_node_def, , "", "Failed to duplicate node")
                    m_unsavedChanges = true;
                }
                
                ImGui::SeparatorText("Comment Assignment");

                // Find all comment nodes in the editor (excluding the current node if it's a comment)
                auto& allNodes = m_editor.getNodes();
                std::vector<std::shared_ptr<comment_node>> comment_nodes;
                std::vector<std::shared_ptr<comment_node>> containingComments;

                for (auto& [id, nodePtr] : allNodes) {
                    if (auto loc_comment_node = std::dynamic_pointer_cast<comment_node>(nodePtr)) {
                        // Don't allow a comment to contain itself
                        if (loc_comment_node.get() == node) {
                            continue;
                        }
                        
                        comment_nodes.push_back(loc_comment_node);
                        // Check if this node is already in the comment
                        if (loc_comment_node->get_contained_nodes().count(node->getUID()) > 0) {
                            containingComments.push_back(loc_comment_node);
                        }
                    }
                }

                if (!comment_nodes.empty()) {
                    // Show which comments already contain this node
                    if (!containingComments.empty()) {
                        ImGui::TextDisabled("Currently in:");
                        for (auto& comment_node : containingComments) {
                            if (ImGui::MenuItem(("Remove from: " + comment_node->getCommentText()).c_str())) {
                                comment_node->remove_contained_node(node->getUID());
                                m_unsavedChanges = true;
                            }
                        }
                        ImGui::Separator();
                    }
                    
                    ImGui::TextDisabled("Add to comment:");
                    for (auto& comment_node : comment_nodes) {
                        // Don't show comments that already contain this node
                        if (comment_node->get_contained_nodes().count(node->getUID()) == 0) {
                            std::string menuText = comment_node->getCommentText();
                            if (menuText.length() > 30) {
                                menuText = menuText.substr(0, 27) + "...";
                            }
                            if (ImGui::MenuItem(menuText.c_str())) {
                                comment_node->add_contained_node(node->getUID());
                                m_unsavedChanges = true;
                            }
                        }
                    }
                } else {
                    ImGui::TextDisabled("No comment nodes available");
                }
            }
        });
    }


    void visual_programming_editor::update(const f32 delta_time) {

        // Currently nothing to update per frame, but can add animation or other logic here
        (void)delta_time; // Prevent unused parameter warning
        
        // Check if any comment node is in adding mode and mark unsaved changes when nodes are added
        auto& nodes = m_editor.getNodes();
        for (auto& [id, node] : nodes) {
            if (auto comment = std::dynamic_pointer_cast<comment_node>(node)) {
                // If a comment was in adding mode but just exited it, nodes were likely added
                static std::set<ImFlow::NodeUID> previouslyInAddMode;
                
                // Check if this comment is currently in adding mode
                // We'll need to add a method to check this - let's add it to comment_node
                // For now, we'll handle this through the node selection logic in comment_node
            }
        }
    }


    void visual_programming_editor::draw(const f32 delta_time) {
        
        if (!m_initialized) return;
            
        try {
            m_editor.update();
        } catch (const std::exception& e) {
            LOG(Error, "Error in editor update: " << e.what())
            // Try to recover by reinitializing
            m_initialized = false;
            initialize();
        }
    }


    void visual_programming_editor::set_size(const ImVec2 new_size)     { m_editor.setSize(new_size); }


    void visual_programming_editor::update_grid_color(glm::vec4 background_color, glm::vec4 grid_color, glm::vec4 subgrid_color) {

        #define COLOR_TO_IM_INT32(var) IM_COL32(var.x * 255, var.y * 255, var.z * 255, var.w * 255)
    
        auto& style = m_editor.getStyle();
        style.colors.background = COLOR_TO_IM_INT32(background_color);
        style.colors.grid = COLOR_TO_IM_INT32(grid_color);
        style.colors.subGrid = COLOR_TO_IM_INT32(subgrid_color);
    
        // Also update the context background color
        m_editor.getGrid().config().color = style.colors.background;
        #undef COLOR_TO_IM_INT32
    }


    void visual_programming_editor::new_graph() {

        if (m_unsavedChanges) {
            // TODO: Add confirmation dialog
        }

        // Clear existing nodes and links properly
        auto& nodes = m_editor.getNodes();
        nodes.clear();
        
        // Clear links
        auto links = m_editor.getLinks();
        for (auto& weak_link : links) {
            if (auto link = weak_link.lock()) {
                // Links will be automatically cleaned up when nodes are destroyed
            }
        }
        
        // Re-initialize with default setup
        initialize();
        m_unsavedChanges = false;
        m_currentFile.clear();
    }


    void visual_programming_editor::load(const std::filesystem::path& filename) {
        if (!std::filesystem::exists(filename)) {
            // Handle error - file doesn't exist
            return;
        }

        m_editor.load(filename, m_node_factories);
        m_currentFile = filename;
        m_unsavedChanges = false;
        
        // Ensure comment nodes update their bounds after loading
        auto& nodes = m_editor.getNodes();
        for (auto& [id, node] : nodes) {
            if (auto comment = std::dynamic_pointer_cast<comment_node>(node)) {
                comment->updateCommentBounds();
            }
        }
    }


    void visual_programming_editor::save(const std::filesystem::path& filename) {
        m_editor.save(filename);
        m_currentFile = filename;
        m_unsavedChanges = false;
    }


    void visual_programming_editor::save_as(const std::filesystem::path& filename) {
        save(filename);
    }


    void visual_programming_editor::draw_menu_bar() {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                new_graph();
            }
            
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // TODO: Implement file dialog - for now use fixed path
                std::filesystem::path open_path = util::get_executable_path() / "visual_programming_test" / "saved_graph.yaml";
                if (std::filesystem::exists(open_path)) {
                    load(open_path);
                }
            }
            
            if (ImGui::MenuItem("Save", "Ctrl+S", false, !m_currentFile.empty())) {
                save(m_currentFile);
            }
            
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                // TODO: Implement file dialog - for now use fixed path
                std::filesystem::path save_path = util::get_executable_path() / "visual_programming_test" / "saved_graph.yaml";
                save_as(save_path);
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Quit", "Alt+F4")) {
                // This would be handled by the dashboard
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Select All", "Ctrl+A")) {
                // Select all nodes
                auto& nodes = m_editor.getNodes();
                for (auto& [id, node] : nodes) {
                    node->selected(true);
                }
            }
            
            if (ImGui::MenuItem("Delete Selected", "Del")) {
                // Delete selected nodes
                auto& nodes = m_editor.getNodes();
                for (auto it = nodes.begin(); it != nodes.end(); ) {
                    if (it->second->isSelected()) {
                        it = nodes.erase(it);
                        m_unsavedChanges = true;
                    } else {
                        ++it;
                    }
                }
            }
            
            ImGui::EndMenu();
        }
        
        // Show current file and unsaved changes indicator
        ImGui::SameLine(ImGui::GetWindowWidth() - 200);
        std::string status = m_currentFile.empty() ? "Untitled" : m_currentFile.filename().string();
        if (m_unsavedChanges) status += " *";
        ImGui::Text("%s", status.c_str());
    }


    void visual_programming_editor::draw_settings_panel() {
        
        // Initialize static variables with current colors
        static glm::vec4 s_background_color  {1.f, 1.f, 1.f, 0.01f};
        static glm::vec4 s_grid_color        {1.f, 1.f, 1.f, 0.10f};
        static glm::vec4 s_sub_grid_color    {1.f, 1.f, 1.f, 0.05f};

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

        
    bool visual_programming_editor::on_key_event(key_event& event) {
        // Check if 'C' key is pressed
        if (event.m_keycode == key_code::key_C && event.m_key_state == key_state::press) {

            auto& nodes = m_editor.getNodes();
            std::vector<std::shared_ptr<ImFlow::BaseNode>> selected_nodes;
            
            for (auto& [id, node] : nodes) {              // Get all selected nodes
                if (node->isSelected())
                    selected_nodes.push_back(node);
            }
            
            if (!selected_nodes.empty()) {      // If we have selected nodes, create a comment around them

                auto new_comment_node = m_editor.placeNode<comment_node>();
                
                // Check if we're selecting any existing comments
                bool hasExistingComments = false;
                for (auto& node : selected_nodes) {
                    if (std::dynamic_pointer_cast<comment_node>(node)) {
                        hasExistingComments = true;
                        break;
                    }
                }
                
                if (hasExistingComments) {
                    // If we're nesting comments, set a different default color
                    new_comment_node->setColor(ImVec4(0.3f, 0.7f, 0.3f, 1.0f)); // Green tint for nested comments
                    new_comment_node->setCommentText("Nested Comment");
                }
                
                for (auto& node : selected_nodes) {                              // Add all selected nodes to the comment
                    new_comment_node->add_contained_node(node->getUID());
                }
                
                m_unsavedChanges = true;
                return true; // Event handled
            }
        }
        
        return false; // Event not handled
    }

}
