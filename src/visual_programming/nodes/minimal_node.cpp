
#include "util/pch.h"

#include <imgui/imgui.h>

#include "minimal_node.h"


namespace AT
{

    
    // Default constructor
    minimal_node::minimal_node(const std::string& name, const std::vector<pin_data_type>& input_types, const std::vector<pin_data_type>& output_types,
        const operation_function& operation)
        : m_name(name), m_operation(operation) {
        
        setTitle(name);
        setStyle(create_minimal_node_style());       // Use a simple style without header
        setup_pins(input_types, output_types);

        // calc node size
        const u32 max_count = math::max((u32)m_input_pins.size(), (u32)m_output_pins.size());
        const auto text_size = ImGui::CalcTextSize(m_name.c_str()).x;
        m_node_size = {50.f + text_size, max_count * 20.f};
    }


    void minimal_node::customDraw(ImDrawList *draw_list, const ImVec2 &offset) {
        
        auto handler = getHandler();
        if (!handler)
            return;

        ImVec2 screenPos = handler->grid2screen(getPos());
        ImVec2 size = getSize();

        // Draw main body (simple rectangle)
        ImU32 bgColor = IM_COL32(60, 60, 60, 255);
        const auto style = getStyle();
        const auto selectedBorder = style->border_selected_color;
        ImU32 borderColor = isSelected() ? IM_COL32(selectedBorder.x * 255, selectedBorder.y * 255, selectedBorder.z * 255, selectedBorder.w * 255) : style->border_color;

        draw_list->AddRectFilled(screenPos, screenPos + size, bgColor, 4.0f);
        draw_list->AddRect(screenPos, screenPos + size, borderColor, 4.0f, 0, 2.0f);

        // Draw node name in center
        ImVec2 textSize = ImGui::CalcTextSize(m_name.c_str());
        ImVec2 textPos = screenPos + (size - textSize) * 0.5f;
        draw_list->AddText(textPos, IM_COL32(255, 255, 255, 255), m_name.c_str());

        // Draw input sockets on left side and handle interactions
        m_hovered_pin_index = -1; // Reset hover state
        float inputSpacing = size.y / (m_input_pins.size() + 1);
        for (size_t i = 0; i < m_input_pins.size(); ++i) {

            if (!m_input_pins[i].pin_ptr)
                continue;

            const ImVec2 socketPos = screenPos + ImVec2(4.0f, inputSpacing * (i + 1));
            m_input_pins[i].pin_ptr->setPos(socketPos);
            draw_pin_socket(draw_list, socketPos, m_input_pins[i], true);

            if (!is_pin_hovered(socketPos, 4.0f))
                continue;

            m_hovered_pin_index = static_cast<int>(i);
            m_hovered_pin_is_input = true;
            auto hoveredPin = get_hovered_pin();
            if (!hoveredPin || !handler)
                continue;

            handler->hovering(hoveredPin);
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                handler->consumeSingleUseClick();           // Let the base system handle pin dragging

            m_input_pins[i].pin_ptr->update();
        }

        // Draw output sockets on right side and handle interactions
        float outputSpacing = size.y / (m_output_pins.size() + 1);
        for (size_t i = 0; i < m_output_pins.size(); ++i) {

            if (!m_output_pins[i].pin_ptr)
                continue;

            const ImVec2 socketPos = screenPos + ImVec2(size.x - 4.0f, outputSpacing * (i + 1));
            const ImVec2 gridPos = handler->screen2grid(socketPos);
            m_output_pins[i].pin_ptr->setPos(gridPos);
            draw_pin_socket(draw_list, socketPos, m_output_pins[i], false);

            if (!is_pin_hovered(socketPos, 4.0f))
                continue;

            m_hovered_pin_index = static_cast<int>(i);
            m_hovered_pin_is_input = false;
            auto hoveredPin = get_hovered_pin();
            if (!hoveredPin || !handler)
                continue;

            handler->hovering(hoveredPin);
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                handler->consumeSingleUseClick();           // Let the base system handle pin dragging

            m_output_pins[i].pin_ptr->update();
        }
    }


    ImFlow::Pin* minimal_node::get_hovered_pin() const {
        
        if (m_hovered_pin_index >= 0) {
            if (m_hovered_pin_is_input && m_hovered_pin_index < static_cast<int>(m_input_pins.size()))
                return m_input_pins[m_hovered_pin_index].pin_ptr.get();

            else if (!m_hovered_pin_is_input && m_hovered_pin_index < static_cast<int>(m_output_pins.size()))
                return m_output_pins[m_hovered_pin_index].pin_ptr.get();
        }
        return nullptr;
    }


    bool minimal_node::customIsHovered() const {
        
        // Consider the node hovered if mouse is over node body OR any pin
        auto handler = getHandler();
        if (!handler)
            return false;

        ImVec2 pos = getPos();
        ImVec2 size = getSize();
        ImVec2 screenMin = handler->grid2screen(pos);
        ImVec2 screenMax = screenMin + size;

        return ImGui::IsMouseHoveringRect(screenMin, screenMax) && !is_mouse_over_any_pin();
    }


    const ImVec2& minimal_node::getSize() const                     { return m_node_size; }


    ImVec2 minimal_node::getVisualSize() const                      { return getSize(); }


    void minimal_node::serialize(AT::serializer::yaml &yaml)        { BaseNode::serialize(yaml); }

    // =============================================================================================
    // PRIVATE
    // =============================================================================================

    void minimal_node::setup_pins(const std::vector<pin_data_type> &input_types, const std::vector<pin_data_type> &output_types) {
        
        // Create input pins
        for (size_t i = 0; i < input_types.size(); ++i) {

            minimal_pin pin;
            pin.type = input_types[i];
            pin.name = "in_" + std::to_string(i);
            pin.pin_ptr = create_typed_pin(pin.name, pin.type, true);
            pin.pin_ptr->renderer([](ImFlow::Pin *p) { /* Empty - we'll draw custom sockets in customDraw, but the base system still handles interactions */ });
            m_input_pins.push_back(pin);
        }

        // Create output pins
        for (size_t i = 0; i < output_types.size(); ++i) {

            minimal_pin pin;
            pin.type = output_types[i];
            pin.name = "out_" + std::to_string(i);
            pin.pin_ptr = create_typed_pin(pin.name, pin.type, false);
            pin.pin_ptr->renderer([](ImFlow::Pin *p) { /* Empty - we'll draw custom sockets in customDraw, but the base system still handles interactions */ });
            m_output_pins.push_back(pin);
        }
    }


    // TODO: move to utils and make generic so any node can use it
    std::shared_ptr<ImFlow::Pin> minimal_node::create_typed_pin(const std::string &name, pin_data_type type, bool is_input) {
        
        auto filter = ImFlow::ConnectionFilter::SameType();

        // Generic lambda that handles all types
        auto create_pin = [&](auto type_tag) -> std::shared_ptr<ImFlow::Pin> {
            using T = typename decltype(type_tag)::type;
            
            if (is_input) {
                return addIN<T>(name, pin_default_value<type>, filter, createPinStyle(type));
            } else {
                auto outPin = addOUT<T>(name, createPinStyle(type));
                outPin->behaviour([this, name]() { 
                    return compute_output<T>(name); 
                });
                return outPin;
            }
        };

        // Switch statement that calls the generic lambda with the appropriate type
        switch (type) {
            case pin_data_type::integer_8:    return create_pin(std::type_identity<int8_t>{});
            case pin_data_type::integer_16:   return create_pin(std::type_identity<int16_t>{});
            case pin_data_type::integer_32:   return create_pin(std::type_identity<int32_t>{});
            case pin_data_type::integer_64:   return create_pin(std::type_identity<int64_t>{});
            
            case pin_data_type::uinteger_8:   return create_pin(std::type_identity<uint8_t>{});
            case pin_data_type::uinteger_16:  return create_pin(std::type_identity<uint16_t>{});
            case pin_data_type::uinteger_32:  return create_pin(std::type_identity<uint32_t>{});
            case pin_data_type::uinteger_64:  return create_pin(std::type_identity<uint64_t>{});
            
            case pin_data_type::float_32:     return create_pin(std::type_identity<float>{});
            case pin_data_type::float_64:     return create_pin(std::type_identity<double>{});
            
            case pin_data_type::string:       return create_pin(std::type_identity<std::string>{});
            case pin_data_type::boolean:      return create_pin(std::type_identity<bool>{});
            
            case pin_data_type::execution:    return nullptr; // Minimal nodes don't use execution pins
        }
        
        return nullptr;
    }

        
    bool minimal_node::is_pin_hovered(const ImVec2& center, float radius) const {
        
        auto handler = getHandler();
        if (!handler)
            return false;

        ImVec2 mousePos = ImGui::GetMousePos();
        float distanceSq = (mousePos.x - center.x) * (mousePos.x - center.x) + (mousePos.y - center.y) * (mousePos.y - center.y);
        
        // Use a slightly larger radius for hover detection than visual radius
        float hoverRadius = radius + 2.0f;
        return distanceSq <= (hoverRadius * hoverRadius);
    }


    float minimal_node::get_hover_radius(const ImVec2& center, float baseRadius) const {
        
        if (is_pin_hovered(center, baseRadius))
            return baseRadius * 1.3f;   // 30% larger when hovered
        
        return baseRadius;
    }


    // Update the draw_pin_socket method to use hover detection
    void minimal_node::draw_pin_socket(ImDrawList *draw_list, const ImVec2 &center, minimal_pin& pin, bool is_input) {
        
        float baseRadius = 4.0f;
        float radius = get_hover_radius(center, baseRadius);
        ImU32 color = pin_data_type_to_color(pin.type);

        if (pin.pin_ptr->isConnected())
            draw_list->AddCircleFilled(center, radius, color);

        else
            draw_list->AddCircle(center, radius, color);
    }


    bool minimal_node::is_mouse_over_any_pin() const {
        
        auto handler = getHandler();
        if (!handler)
            return false;

        ImVec2 screenPos = handler->grid2screen(getPos());
        ImVec2 size = getSize();

        // Check input pins
        float inputSpacing = size.y / (m_input_pins.size() + 1);
        for (size_t i = 0; i < m_input_pins.size(); ++i) {

            ImVec2 socketPos = screenPos + ImVec2(4.0f, inputSpacing * (i + 1));
            if (is_pin_hovered(socketPos, 4.0f))
                return true;
        }

        // Check output pins
        float outputSpacing = size.y / (m_output_pins.size() + 1);
        for (size_t i = 0; i < m_output_pins.size(); ++i) {

            ImVec2 socketPos = screenPos + ImVec2(size.x - 4.0f, outputSpacing * (i + 1));
            if (is_pin_hovered(socketPos, 4.0f))
                return true;
        }

        return false;
    }


    bool minimal_node::is_mouse_over_node_body() const {

        auto handler = getHandler();
        if (!handler)
            return false;

        ImVec2 pos = getPos();
        ImVec2 size = getSize();
        ImVec2 screenMin = handler->grid2screen(pos);
        ImVec2 screenMax = screenMin + size;

        // Check if mouse is within node bounds but NOT over any pin
        return ImGui::IsMouseHoveringRect(screenMin, screenMax) && !is_mouse_over_any_pin();
    }


    std::shared_ptr<ImFlow::NodeStyle> minimal_node::create_minimal_node_style() {

        auto style = std::make_shared<ImFlow::NodeStyle>(
            IM_COL32(0, 0, 0, 0),       // Transparent header
            ImColor(255, 255, 255, 255),
            4.0f                        // Radius for rounded corners
        );
        style->bg = IM_COL32(60, 60, 60, 255);
        style->border_thickness = 2.0f;
        style->border_selected_thickness = 3.0f;
        style->border_color = IM_COL32(100, 100, 100, 255);
        style->border_selected_color = UI::get_main_color_ref();
        style->padding = ImVec4(0.f, 0.f, 0.f, 0.f);

        return style;
    }

}
