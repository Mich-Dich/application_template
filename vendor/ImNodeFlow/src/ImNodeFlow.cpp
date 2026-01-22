#include "ImNodeFlow.h"

namespace ImFlow {

    // -----------------------------------------------------------------------------------------------------------------
    // LINK

    void Link::update() {
        ImVec2 start = m_left->pinPoint();
        ImVec2 end = m_right->pinPoint();
        float thickness = m_left->getStyle()->extra.link_thickness;
        bool mouseClickState = m_inf->getSingleUseClick();

        if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            m_selected = false;

        // Replace smart_bezier_collider with subway_connection_collider
        if (subway_connection_collider(ImGui::GetMousePos(), start, end, 2.5)) {
            m_hovered = true;
            thickness = m_left->getStyle()->extra.link_hovered_thickness;
            if (mouseClickState) {
                m_inf->consumeSingleUseClick();
                m_selected = true;
            }
        } else { m_hovered = false; }

        if (m_selected)
            // Replace smart_bezier with subway_connection for outline
            subway_connection(start, end, m_left->getStyle()->extra.outline_color,
                        thickness + m_left->getStyle()->extra.link_selected_outline_thickness);
        
        // Replace smart_bezier with subway_connection for main line
        subway_connection(start, end, m_left->getStyle()->color, thickness);

        if (m_selected && ImGui::IsKeyPressed(ImGuiKey_Delete, false))
            m_right->deleteLink();
    }


    Link::~Link() {
        m_left->deleteLink();
    }

    // -----------------------------------------------------------------------------------------------------------------
    // SUBWAY STYLE CONNECTIONS

    // Helper function to calculate distance from point to line segment
    inline static float ImPointSegmentDistance(const ImVec2& p, const ImVec2& a, const ImVec2& b) {
        ImVec2 ab = b - a;
        ImVec2 ap = p - a;
        
        float ab_length_sq = ab.x * ab.x + ab.y * ab.y;
        if (ab_length_sq < 1e-6f) {
            // a and b are the same point
            return ImLength(ap);
        }
        
        // Project point onto the segment
        float t = ImClamp(ImDot(ap, ab) / ab_length_sq, 0.0f, 1.0f);
        ImVec2 projection = a + t * ab;
        
        return ImLength(p - projection);
    }


    // -----------------------------------------------------------------------------------------------------------------
    // SUBWAY STYLE CONNECTIONS - SMART 45° ROUTING

    inline static void subway_connection(const ImVec2& p1, const ImVec2& p2, ImU32 color, float thickness) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        
        ImVec2 delta = p2 - p1;
        float abs_dx = fabsf(delta.x);
        float abs_dy = fabsf(delta.y);
        
        // Choose routing based on direction to create nice 45° diagonals
        if (delta.x * delta.y > 0) {
            // Same direction (bottom-right or top-left)
            // Use two 45° segments with a diagonal in the middle
            float diagonal_length = ImMin(abs_dx, abs_dy);
            float remaining = (ImMax(abs_dx, abs_dy) - diagonal_length) * 0.5f;
            
            if (abs_dx > abs_dy) {
                ImVec2 p1_horiz = p1 + ImVec2(delta.x > 0 ? remaining : -remaining, 0);
                ImVec2 p2_diag = p1_horiz + ImVec2(delta.x > 0 ? diagonal_length : -diagonal_length, 
                                                delta.y > 0 ? diagonal_length : -diagonal_length);
                draw_list->AddLine(p1, p1_horiz, color, thickness);
                draw_list->AddLine(p1_horiz, p2_diag, color, thickness);
                draw_list->AddLine(p2_diag, p2, color, thickness);
            } else {
                ImVec2 p1_vert = p1 + ImVec2(0, delta.y > 0 ? remaining : -remaining);
                ImVec2 p2_diag = p1_vert + ImVec2(delta.x > 0 ? diagonal_length : -diagonal_length,
                                                delta.y > 0 ? diagonal_length : -diagonal_length);
                draw_list->AddLine(p1, p1_vert, color, thickness);
                draw_list->AddLine(p1_vert, p2_diag, color, thickness);
                draw_list->AddLine(p2_diag, p2, color, thickness);
            }
        } else {
            // Opposite direction (bottom-left or top-right)
            // Use a single 45° diagonal in the middle
            float diagonal_length = ImMin(abs_dx, abs_dy);
            
            if (abs_dx > abs_dy) {
                float remaining = (abs_dx - diagonal_length) * 0.5f;
                ImVec2 p1_horiz = p1 + ImVec2(delta.x > 0 ? remaining : -remaining, 0);
                ImVec2 p2_diag = p1_horiz + ImVec2(delta.x > 0 ? diagonal_length : -diagonal_length, delta.y);
                draw_list->AddLine(p1, p1_horiz, color, thickness);
                draw_list->AddLine(p1_horiz, p2_diag, color, thickness);
                draw_list->AddLine(p2_diag, p2, color, thickness);
            } else {
                float remaining = (abs_dy - diagonal_length) * 0.5f;
                ImVec2 p1_vert = p1 + ImVec2(0, delta.y > 0 ? remaining : -remaining);
                ImVec2 p2_diag = p1_vert + ImVec2(delta.x, delta.y > 0 ? diagonal_length : -diagonal_length);
                draw_list->AddLine(p1, p1_vert, color, thickness);
                draw_list->AddLine(p1_vert, p2_diag, color, thickness);
                draw_list->AddLine(p2_diag, p2, color, thickness);
            }
        }
    }


    inline static bool subway_connection_collider(const ImVec2& p, const ImVec2& p1, const ImVec2& p2, float radius) {
        ImVec2 delta = p2 - p1;
        float abs_dx = fabsf(delta.x);
        float abs_dy = fabsf(delta.y);
        
        // Check collision with each segment based on the connection type
        if (abs_dx > abs_dy) {
            // Horizontal-diagonal-horizontal pattern
            float diagonal_length = abs_dy;
            float remaining_horizontal = (abs_dx - diagonal_length) * 0.5f;
            
            ImVec2 p1_horizontal = p1 + ImVec2(delta.x > 0 ? remaining_horizontal : -remaining_horizontal, 0);
            ImVec2 p2_diagonal = p1_horizontal + ImVec2(delta.x > 0 ? diagonal_length : -diagonal_length, delta.y);
            
            // Check first horizontal segment
            if (ImPointSegmentDistance(p, p1, p1_horizontal) <= radius) return true;
            // Check diagonal segment  
            if (ImPointSegmentDistance(p, p1_horizontal, p2_diagonal) <= radius) return true;
            // Check second horizontal segment
            if (ImPointSegmentDistance(p, p2_diagonal, p2) <= radius) return true;
        } else {
            // Vertical-diagonal-vertical pattern
            float diagonal_length = abs_dx;
            float remaining_vertical = (abs_dy - diagonal_length) * 0.5f;
            
            ImVec2 p1_vertical = p1 + ImVec2(0, delta.y > 0 ? remaining_vertical : -remaining_vertical);
            ImVec2 p2_diagonal = p1_vertical + ImVec2(delta.x, delta.y > 0 ? diagonal_length : -diagonal_length);
            
            // Check first vertical segment
            if (ImPointSegmentDistance(p, p1, p1_vertical) <= radius) return true;
            // Check diagonal segment
            if (ImPointSegmentDistance(p, p1_vertical, p2_diagonal) <= radius) return true;
            // Check second vertical segment
            if (ImPointSegmentDistance(p, p2_diagonal, p2) <= radius) return true;
        }
        
        return false;
    }

    // -----------------------------------------------------------------------------------------------------------------
    // BASE NODE

    bool BaseNode::isHovered() {
        // For custom nodes, use the custom hover detection if available
        if (usesCustomHover()) {
            return customIsHovered();
        }
        
        // Original logic for regular nodes
        ImVec2 paddingTL = {m_style->padding.x, m_style->padding.y};
        ImVec2 paddingBR = {m_style->padding.z, m_style->padding.w};
        return ImGui::IsMouseHoveringRect(m_inf->grid2screen(m_pos - paddingTL),
                                        m_inf->grid2screen(m_pos + m_size + paddingBR));
    }


    void BaseNode::update() {

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImGui::PushID(this);
        bool mouseClickState = m_inf->getSingleUseClick();
        ImVec2 offset = m_inf->grid2screen({0.f, 0.f});
        ImVec2 paddingTL = {m_style->padding.x, m_style->padding.y};
        ImVec2 paddingBR = {m_style->padding.z, m_style->padding.w};


        // Check if this node has custom drawing
        if (usesCustomHover()) {
            // For custom nodes, use custom drawing
            customDraw(draw_list, offset);
            
            // Get fresh mouse state for this frame
            bool mouseClickState = m_inf->getSingleUseClick();
            
            // Handle selection
            if (ImGui::IsWindowHovered() && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl) &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_inf->on_selected_node())
                selected(false);

            // Hover detection for custom nodes
            if (isHovered()) {
                m_inf->hoveredNode(this);
                if (mouseClickState) {
                    selected(true);
                    m_inf->consumeSingleUseClick();
                    
                    // Start dragging when clicking on a custom node
                    if (!m_inf->isBoxSelecting()) {
                        m_dragged = true;
                        m_inf->draggingNode(true);
                    }
                }
            }

            // Handle deletion
            if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete) && 
                !ImGui::IsAnyItemActive() && isSelected())
                destroy();

            // Handle dragging for custom nodes
            if (m_dragged || (isSelected() && m_inf->isNodeDragged())) {
                float step = m_inf->getStyle().grid_size / m_inf->getStyle().grid_subdivisions;
                m_posTarget += ImGui::GetIO().MouseDelta;
                // "Slam" The position to grid
                m_pos.x = round(m_posTarget.x / step) * step;
                m_pos.y = round(m_posTarget.y / step) * step;

                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    m_dragged = false;
                    m_inf->draggingNode(false);
                    m_posTarget = m_pos;
                }
            }
            
            ImGui::PopID();
            return;
        }

        draw_list->ChannelsSetCurrent(1); // Foreground
        ImGui::SetCursorScreenPos(offset + m_pos);

        ImGui::BeginGroup();

        // Header
        ImGui::BeginGroup();
        ImGui::TextColored(m_style->header_title_color, "%s", m_title.c_str());
        ImGui::Spacing();
        ImGui::EndGroup();
        float headerH = ImGui::GetItemRectSize().y;
        float titleW = ImGui::GetItemRectSize().x;

        // Inputs
        if (!m_ins.empty() || !m_dynamicIns.empty()) {
            ImGui::BeginGroup();
            for (auto &p: m_ins) {
                p->setPos(ImGui::GetCursorPos());
                p->update();
            }
            for (auto &p: m_dynamicIns) {
                if (p.first == 1) {
                    p.second->setPos(ImGui::GetCursorPos());
                    p.second->update();
                    p.first = 0;
                }
            }
            ImGui::EndGroup();
            ImGui::SameLine();
        }

        // Content
        ImGui::BeginGroup();
        draw();
        ImGui::Dummy(ImVec2(0.f, 0.f));
        ImGui::EndGroup();
        ImGui::SameLine();

        // Outputs
        float maxW = 0.0f;
        for (auto &p: m_outs) {
            float w = p->calcWidth();
            if (w > maxW)
                maxW = w;
        }
        for (auto &p: m_dynamicOuts) {
            float w = p.second->calcWidth();
            if (w > maxW)
                maxW = w;
        }
        ImGui::BeginGroup();
        for (auto &p: m_outs) {
            // FIXME: This looks horrible
            if ((m_pos + ImVec2(titleW, 0) + m_inf->getGrid().scroll()).x <
                ImGui::GetCursorPos().x + ImGui::GetWindowPos().x + maxW)
                p->setPos(ImGui::GetCursorPos() + ImGui::GetWindowPos() + ImVec2(maxW - p->calcWidth(), 0.f));
            else
                p->setPos(ImVec2((m_pos + ImVec2(titleW - p->calcWidth(), 0) + m_inf->getGrid().scroll()).x,
                                 ImGui::GetCursorPos().y + ImGui::GetWindowPos().y));
            p->update();
        }
        for (auto &p: m_dynamicOuts) {
            // FIXME: This looks horrible
            if ((m_pos + ImVec2(titleW, 0) + m_inf->getGrid().scroll()).x < ImGui::GetCursorPos().x + ImGui::GetWindowPos().x + maxW)
                p.second->setPos(ImGui::GetCursorPos() + ImGui::GetWindowPos() + ImVec2(maxW - p.second->calcWidth(), 0.f));
            else
                p.second->setPos(ImVec2((m_pos + ImVec2(titleW - p.second->calcWidth(), 0) + m_inf->getGrid().scroll()).x,
                               ImGui::GetCursorPos().y + ImGui::GetWindowPos().y));
            p.second->update();
            p.first -= 1;
        }

        ImGui::EndGroup();

        ImGui::EndGroup();
        m_size = ImGui::GetItemRectSize();
    
        // Force node size to be multiples of grid step for alignment
        float gridStep = m_inf->getStyle().grid_size / m_inf->getStyle().grid_subdivisions;
        m_size.x = ceil(m_size.x / gridStep) * gridStep;
        m_size.y = ceil(m_size.y / gridStep) * gridStep;
        
        ImVec2 headerSize = ImVec2(m_size.x + paddingBR.x, headerH);

        // Background
        draw_list->ChannelsSetCurrent(0);
        draw_list->AddRectFilled(offset + m_pos - paddingTL, offset + m_pos + m_size + paddingBR, m_style->bg, m_style->radius);
        draw_list->AddRectFilled(offset + m_pos - paddingTL, offset + m_pos + headerSize, m_style->header_bg, m_style->radius, ImDrawFlags_RoundCornersTop);
        m_fullSize = m_size + paddingTL + paddingBR;
        ImU32 col = m_style->border_color;
        float thickness = m_style->border_thickness;
        ImVec2 ptl = paddingTL;
        ImVec2 pbr = paddingBR;
        if (m_selected) {
            col = IM_COL32(m_style->border_selected_color.x * 255, m_style->border_selected_color.y * 255, m_style->border_selected_color.z * 255, m_style->border_selected_color.w * 255);
            thickness = m_style->border_selected_thickness;
        }
        if (thickness < 0.f) {
            ptl.x -= thickness / 2;
            ptl.y -= thickness / 2;
            pbr.x -= thickness / 2;
            pbr.y -= thickness / 2;
            thickness *= -1.f;
        }
        draw_list->AddRect(offset + m_pos - ptl, offset + m_pos + m_size + pbr, col, m_style->radius, 0, thickness);


        if (ImGui::IsWindowHovered() && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl) &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !m_inf->on_selected_node())
            selected(false);

        if (isHovered()) {
            m_inf->hoveredNode(this);
            if (mouseClickState) {
                selected(true);
                m_inf->consumeSingleUseClick();
            }
        }

        if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Delete) && !ImGui::IsAnyItemActive() && isSelected())
            destroy();

        bool onHeader = ImGui::IsMouseHoveringRect(offset + m_pos - paddingTL, offset + m_pos + headerSize);
        if (onHeader && mouseClickState) {
            m_inf->consumeSingleUseClick();
            m_dragged = true;
            m_inf->draggingNode(true);
        }
        if (m_dragged || (m_selected && m_inf->isNodeDragged())) {
            float step = m_inf->getStyle().grid_size / m_inf->getStyle().grid_subdivisions;
            m_posTarget += ImGui::GetIO().MouseDelta;
            // "Slam" The position
            m_pos.x = round(m_posTarget.x / step) * step;
            m_pos.y = round(m_posTarget.y / step) * step;

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                m_dragged = false;
                m_inf->draggingNode(false);
                m_posTarget = m_pos;
            }
        }
        ImGui::PopID();

        // Deleting dead pins
        m_dynamicIns.erase(std::remove_if(m_dynamicIns.begin(), m_dynamicIns.end(),
                                          [](const std::pair<int, std::shared_ptr<Pin>> &p) { return p.first == 0; }),
                           m_dynamicIns.end());
        m_dynamicOuts.erase(std::remove_if(m_dynamicOuts.begin(), m_dynamicOuts.end(),
                                           [](const std::pair<int, std::shared_ptr<Pin>> &p) { return p.first == 0; }),
                            m_dynamicOuts.end());
    }

    // -----------------------------------------------------------------------------------------------------------------
    // HANDLER

    
    int ImNodeFlow::m_instances = 0;


    bool ImNodeFlow::on_selected_node() {
        return std::any_of(m_nodes.begin(), m_nodes.end(),
                           [](const auto &n) { return n.second->isSelected() && n.second->isHovered(); });
    }


    bool ImNodeFlow::on_free_space() {
        // Check if any node is hovered (including custom nodes)
        bool anyNodeHovered = std::any_of(m_nodes.begin(), m_nodes.end(),
            [](const auto &n) { return n.second->isHovered(); });
        
        // Check if any link is hovered
        bool anyLinkHovered = std::any_of(m_links.begin(), m_links.end(),
            [](const auto &l) { 
                auto link = l.lock();
                return link && link->isHovered(); 
            });
        
        // Free space means no nodes AND no links are hovered
        return !anyNodeHovered && !anyLinkHovered;
    }


    ImVec2 ImNodeFlow::screen2grid( const ImVec2 & p )
    {
        if ( ImGui::GetCurrentContext() == m_context.getRawContext() )
            return p - m_context.scroll();
        return ( p - m_context.origin() ) / m_context.scale() - m_context.scroll();
    }


    ImVec2 ImNodeFlow::grid2screen( const ImVec2 & p )
    {
        if ( ImGui::GetCurrentContext() == m_context.getRawContext() )
            return p + m_context.scroll();
        return ( p + m_context.scroll() ) * m_context.scale() + m_context.origin();
    }


    void ImNodeFlow::addLink(std::shared_ptr<Link> &link) {
        m_links.push_back(link);
    }


    void ImNodeFlow::update() {
        // Updating looping stuff
        m_hovering = nullptr;
        m_hoveredNode = nullptr;
        m_draggingNode = m_draggingNodeNext;
        m_singleUseClick = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

        // Create child canvas
        m_context.begin();
        ImGui::GetIO().IniFilename = nullptr;

        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        // Display grid
        ImVec2 gridSize = ImGui::GetWindowSize();
        float subGridStep = m_style.grid_size / m_style.grid_subdivisions;
        for (float x = fmodf(m_context.scroll().x, m_style.grid_size); x < gridSize.x; x += m_style.grid_size)
            draw_list->AddLine(ImVec2(x, 0.0f), ImVec2(x, gridSize.y), m_style.colors.grid);
        for (float y = fmodf(m_context.scroll().y, m_style.grid_size); y < gridSize.y; y += m_style.grid_size)
            draw_list->AddLine(ImVec2(0.0f, y), ImVec2(gridSize.x, y), m_style.colors.grid);
        if (m_context.scale() > 0.7f) {
            for (float x = fmodf(m_context.scroll().x, subGridStep); x < gridSize.x; x += subGridStep)
                draw_list->AddLine(ImVec2(x, 0.0f), ImVec2(x, gridSize.y), m_style.colors.subGrid);
            for (float y = fmodf(m_context.scroll().y, subGridStep); y < gridSize.y; y += subGridStep)
                draw_list->AddLine(ImVec2(0.0f, y), ImVec2(gridSize.x, y), m_style.colors.subGrid);
        }

        handleBoxSelection();           // Box selection logic

        // Update and draw nodes
        // TODO: I don't like this
        draw_list->ChannelsSplit(2);
            
        // First pass: Draw custom nodes in background
        for (auto &node: m_nodes) { 
            if (node.second->usesCustomDrawing()) {
                node.second->update();
            }
        }
        
        // Second pass: Draw regular nodes in foreground
        for (auto &node: m_nodes) { 
            if (!node.second->usesCustomDrawing()) {
                node.second->update();
            }
        }
        
        // Remove "toDelete" nodes
        for (auto iter = m_nodes.begin(); iter != m_nodes.end();) {
            if (iter->second->toDestroy())
                iter = m_nodes.erase(iter);
            else
                ++iter;
        }
        draw_list->ChannelsMerge();
        for (auto &node: m_nodes) { node.second->updatePublicStatus(); }

        // Update and draw links
        for (auto &l: m_links) { if (!l.expired()) l.lock()->update(); }

        // Links drop-off
        if (m_dragOut && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            if (!m_hovering) {
                if (on_free_space() && m_droppedLinkPopUp) {
                    if (m_droppedLinkPupUpComboKey == ImGuiKey_None || ImGui::IsKeyDown(m_droppedLinkPupUpComboKey)) {
                        m_droppedLinkLeft = m_dragOut;
                        ImGui::OpenPopup("DroppedLinkPopUp");
                    }
                }
            } else
                m_dragOut->createLink(m_hovering);
        }

        // Links drag-out
        if (!m_draggingNode && m_hovering && !m_dragOut && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            m_dragOut = m_hovering;
        if (m_dragOut) {
            if (m_dragOut->getType() == PinType_Output)
                // Replace smart_bezier with subway_connection
                subway_connection(m_dragOut->pinPoint(), ImGui::GetMousePos(), m_dragOut->getStyle()->color,
                            m_dragOut->getStyle()->extra.link_dragged_thickness);
            else
                // Replace smart_bezier with subway_connection  
                subway_connection(ImGui::GetMousePos(), m_dragOut->pinPoint(), m_dragOut->getStyle()->color,
                            m_dragOut->getStyle()->extra.link_dragged_thickness);

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                m_dragOut = nullptr;
        }

        // Right-click PopUp
        if (m_rightClickPopUp && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && ImGui::IsWindowHovered()) {
            m_hoveredNodeAux = m_hoveredNode;
            ImGui::OpenPopup("RightClickPopUp");
        }
        if (ImGui::BeginPopup("RightClickPopUp")) {
            m_rightClickPopUp(m_hoveredNodeAux);
            ImGui::EndPopup();
        }

        // Dropped Link PopUp
        if (ImGui::BeginPopup("DroppedLinkPopUp")) {
            m_droppedLinkPopUp(m_droppedLinkLeft);
            ImGui::EndPopup();
        }

        // Removing dead Links
        m_links.erase(std::remove_if(m_links.begin(), m_links.end(),
                                     [](const std::weak_ptr<Link> &l) { return l.expired(); }), m_links.end());

        // Clearing recursion blacklist
        m_pinRecursionBlacklist.clear();

        m_context.end();
    }


    void ImNodeFlow::handleBoxSelection() {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        
        // Start box selection (left mouse button on empty space, no modifier keys)
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && on_free_space() && 
            !m_draggingNode && !m_dragOut && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
            m_boxSelecting = true;
            m_boxSelectStart = ImGui::GetMousePos();
            m_boxSelectStartGrid = screen2grid(m_boxSelectStart);
            m_boxSelectEnd = m_boxSelectStart;
        }
        
        // Update box selection while dragging
        if (m_boxSelecting) {
            m_boxSelectEnd = ImGui::GetMousePos();
            
            // Draw selection box
            ImVec2 screenStart = m_boxSelectStart;
            ImVec2 screenEnd = m_boxSelectEnd;
            
            // Ensure proper rectangle coordinates
            ImVec2 rectMin = ImVec2(std::min(screenStart.x, screenEnd.x), std::min(screenStart.y, screenEnd.y));
            ImVec2 rectMax = ImVec2(std::max(screenStart.x, screenEnd.x), std::max(screenStart.y, screenEnd.y));
            
            // Draw filled rectangle with transparency
            draw_list->AddRectFilled(rectMin, rectMax, IM_COL32(100, 100, 100, 30));
            // Draw border
            draw_list->AddRect(rectMin, rectMax, IM_COL32(100, 100, 100, 200), 0.0f, 0, 2.0f);
            
            // Update selection while dragging (optional - for real-time feedback)
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 gridEnd = screen2grid(m_boxSelectEnd);
                ImVec2 selectionMin = ImVec2(std::min(m_boxSelectStartGrid.x, gridEnd.x), 
                                        std::min(m_boxSelectStartGrid.y, gridEnd.y));
                ImVec2 selectionMax = ImVec2(std::max(m_boxSelectStartGrid.x, gridEnd.x), 
                                        std::max(m_boxSelectStartGrid.y, gridEnd.y));
                
                // Select nodes within the box
                for (auto& node : m_nodes) {
                    ImVec2 nodePos = node.second->getPos();
                    ImVec2 nodeSize = node.second->getFullSize();
                    
                    // Check if node is within selection box
                    bool inSelection = (nodePos.x >= selectionMin.x && 
                                    nodePos.x + nodeSize.x <= selectionMax.x &&
                                    nodePos.y >= selectionMin.y && 
                                    nodePos.y + nodeSize.y <= selectionMax.y);
                    
                    node.second->selected(inSelection);
                }
            }
            
            // End box selection
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                ImVec2 gridEnd = screen2grid(m_boxSelectEnd);
                ImVec2 selectionMin = ImVec2(std::min(m_boxSelectStartGrid.x, gridEnd.x), 
                                        std::min(m_boxSelectStartGrid.y, gridEnd.y));
                ImVec2 selectionMax = ImVec2(std::max(m_boxSelectStartGrid.x, gridEnd.x), 
                                        std::max(m_boxSelectStartGrid.y, gridEnd.y));
                
                // Final selection on mouse release
                for (auto& node : m_nodes) {
                    ImVec2 nodePos = node.second->getPos();
                    ImVec2 nodeSize = node.second->getFullSize();
                    
                    // Check if node is within selection box
                    bool inSelection = (nodePos.x >= selectionMin.x && 
                                    nodePos.x + nodeSize.x <= selectionMax.x &&
                                    nodePos.y >= selectionMin.y && 
                                    nodePos.y + nodeSize.y <= selectionMax.y);
                    
                    node.second->selected(inSelection);
                }
                
                m_boxSelecting = false;
            }
        }
        
        // Clear selection when clicking on empty space without dragging (if not box selecting)
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && on_free_space() && 
            !m_boxSelecting && !ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
            for (auto& node : m_nodes) {
                node.second->selected(false);
            }
        }
    }


    void ImNodeFlow::save(const std::filesystem::path& filename) {
        AT::serializer::yaml file_serializer(filename, "node_editor", AT::serializer::option::save_to_file);
        
        // Save nodes
        file_serializer.sub_section("nodes", [this](AT::serializer::yaml& yaml) {
            std::vector<NodeUID> node_uids;
            for (const auto& [uid, node] : m_nodes) {
                node_uids.push_back(uid);
            }
            
            yaml.entry(KEY_VALUE(node_uids));
            
            for (const auto& [uid, node] : m_nodes) {
                yaml.sub_section(std::to_string(uid), [&node](AT::serializer::yaml& node_yaml) {
                    node->serialize(node_yaml);
                    node->serializePins(node_yaml);
                });
            }
        });
        
        // Save links
        std::vector<std::tuple<NodeUID, std::string, NodeUID, std::string>> links;
        
        for (const auto& link_weak : m_links) {
            if (auto link = link_weak.lock()) {
                Pin* left_pin = link->left();
                Pin* right_pin = link->right();
                
                if (left_pin && right_pin) {
                    NodeUID left_node_uid = left_pin->getParent()->getUID();
                    NodeUID right_node_uid = right_pin->getParent()->getUID();
                    
                    std::string left_pin_name = left_pin->getName();
                    std::string right_pin_name = right_pin->getName();
                    
                    links.emplace_back(left_node_uid, left_pin_name, right_node_uid, right_pin_name);
                }
            }
        }
        
        file_serializer.vector(KEY_VALUE(links), [&](AT::serializer::yaml& link_yaml, u64 x) {

            link_yaml.entry("from_node", std::get<0>(links[x]))
                .entry("from_pin", std::get<1>(links[x]))
                .entry("to_node", std::get<2>(links[x]))
                .entry("to_pin", std::get<3>(links[x]));
        });
    }
            
    
    void ImNodeFlow::load(const std::filesystem::path& filename, std::unordered_map<std::string, NodeFactory>& node_factories) {

        // Clear current graph
        m_nodes.clear();
        m_links.clear();
        
        try {
            AT::serializer::yaml file_serializer(filename, "node_editor", AT::serializer::option::load_from_file);
            
            std::unordered_map<NodeUID, std::shared_ptr<BaseNode>> loaded_nodes;
            // Load nodes
            file_serializer.sub_section("nodes", [&](AT::serializer::yaml& yaml) {
                // Read node_count array to get all node UIDs
                std::vector<NodeUID> node_uids;
                yaml.entry(KEY_VALUE(node_uids));
                VALIDATE(!node_uids.empty(), return, "Found [" << node_uids.size() << "] nodes in file", "Failed to read node_count from YAML file")
                
                for (NodeUID uid : node_uids) {
                    std::string uid_str = std::to_string(uid);
                    LOG(Trace, "Loading node with UID [" << uid_str << "]");

                    yaml.sub_section(uid_str, [&](AT::serializer::yaml& node_yaml) {        // Read basic node properties

                        std::string node_type{};
                        ImVec2 position{};
                        std::string title{};
                        bool select = false;
                        
                        node_yaml.entry("position_x", position.x)
                            .entry("position_y", position.y)
                            .entry("title", title)
                            .entry("selected", select)
                            .entry("type", node_type);

                        VALIDATE(!node_type.empty(), , "Creating node: [" << node_type << "] at (" << position.x << ", " << position.y << ")", "Could not load node_type")
                        
                        // Create node based on type
                        auto factory_it = node_factories.find(node_type);
                        if (factory_it != node_factories.end()) {
                            auto node = factory_it->second(position);
                            if (node) {
                                node->setUID(uid); // Set the original UID
                                node->setTitle(title);
                                node->selected(select);
                                
                                // Let node load any additional data
                                node->serialize(node_yaml);
                                node->serializePins(node_yaml);
                                
                                // Add to our temporary map and main nodes list
                                loaded_nodes[uid] = node;
                                
                                LOG(Trace, "Successfully created node: [" << title << "]");
                            } else {
                                LOG(Error, "Factory failed to create node of type: [" << node_type << "]");
                            }
                        } else {
                            LOG(Error, "No factory found for node type: [" << node_type << "]");
                        }
                    });

                }
            });
            
            LOG(Debug, "Successfully loaded [" << loaded_nodes.size() << "] nodes");
            
            // Load links
            std::vector<std::tuple<NodeUID, std::string, NodeUID, std::string>> links;

            // Use the vector reading approach that matches your YAML structure
            file_serializer.vector(KEY_VALUE(links), [&](AT::serializer::yaml& link_yaml, u64 x) {
                
                NodeUID from_node{}, to_node{};
                std::string from_pin{}, to_pin{};

                link_yaml.entry("from_node", from_node)
                    .entry("from_pin", from_pin)
                    .entry("to_node", to_node)
                    .entry("to_pin", to_pin);

                links[x] = {from_node, from_pin, to_node, to_pin};
            });
                        
            LOG(Trace, "Found [" << links.size() << "] links in file");
            
            // Recreate links
            for (const auto& link_data : links) {
                NodeUID from_uid = std::get<0>(link_data);
                std::string from_pin_name = std::get<1>(link_data);
                NodeUID to_uid = std::get<2>(link_data);
                std::string to_pin_name = std::get<3>(link_data);
                
                auto from_node_it = loaded_nodes.find(from_uid);
                auto to_node_it = loaded_nodes.find(to_uid);
                
                if (from_node_it != loaded_nodes.end() && to_node_it != loaded_nodes.end()) {
                    Pin* from_pin = from_node_it->second->outPin(from_pin_name.c_str());
                    Pin* to_pin = to_node_it->second->inPin(to_pin_name.c_str());
                    
                    if (from_pin && to_pin) {
                        from_pin->createLink(to_pin);
                        LOG(Trace, "Created link from [" << from_node_it->second->getName() << "].[" << from_pin_name << "] to [" << to_node_it->second->getName() << "].[" << to_pin_name << "]");
                    } else {
                        if (!from_pin) LOG(Error, "Could not find output pin: [" << from_pin_name << "] on node [" << from_node_it->second->getName() << "]");
                        if (!to_pin) LOG(Error, "Could not find input pin: [" << to_pin_name << "] on node [" << to_node_it->second->getName() << "]");
                    }
                } else {
                    if (from_node_it == loaded_nodes.end()) LOG(Error, "Source node not found [" << from_uid << "]");
                    if (to_node_it == loaded_nodes.end()) LOG(Error, "Target node not found [" << to_uid << "]");
                }
            }
            
            LOG(Trace, "Graph loaded successfully with [" << m_nodes.size() << "] nodes and [" << m_links.size() << "] links");
            
        } catch (const std::exception& e) {
            LOG(Error, "Exception during graph loading [" << e.what() << "]");
        } catch (...) {
            LOG(Error, "Unknown exception during graph loading");
        }
    }

}
