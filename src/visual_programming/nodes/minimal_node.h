#pragma once

#include "utils.h"

namespace AT {

    class minimal_node : public ImFlow::BaseNode {
    public:

        minimal_node(const std::string& name, 
                    const std::vector<pin_data_type>& input_types,
                    const std::vector<pin_data_type>& output_types,
                    const std::function<std::vector<function_value>(const std::vector<function_value>&)>& operation = nullptr)
            : m_name(name), m_operation(operation) {
            
            setTitle(name);
            // Use a simple style without header
            setStyle(create_minimal_node_style());
            
            setupPins(input_types, output_types);
        }
        

        void draw() override {
            // No content in the main body - everything is handled by custom drawing
        }
        

        bool usesCustomDrawing() const override { return true; }
        

        void customDraw(ImDrawList* draw_list, const ImVec2& offset) override {
            auto handler = getHandler();
            if (!handler) return;
            
            ImVec2 screenPos = handler->grid2screen(getPos());
            ImVec2 size = getSize();
            
            // Draw main body (simple rectangle)
            ImU32 bgColor = IM_COL32(60, 60, 60, 255);
            ImU32 borderColor = isSelected() ? IM_COL32(255, 255, 100, 255) : IM_COL32(100, 100, 100, 255);
            
            draw_list->AddRectFilled(screenPos, screenPos + size, bgColor, 4.0f);
            draw_list->AddRect(screenPos, screenPos + size, borderColor, 4.0f, 0, 2.0f);
            
            // Draw node name in center
            ImVec2 textSize = ImGui::CalcTextSize(m_name.c_str());
            ImVec2 textPos = screenPos + (size - textSize) * 0.5f;
            draw_list->AddText(textPos, IM_COL32(255, 255, 255, 255), m_name.c_str());
            
            // Update pin positions for interaction
            updatePinPositions(screenPos, size);
            
            // Draw input sockets on left side (inside the node)
            float inputSpacing = size.y / (m_inputPins.size() + 1);
            for (size_t i = 0; i < m_inputPins.size(); ++i) {
                ImVec2 socketPos = screenPos + ImVec2(4.0f, inputSpacing * (i + 1)); // Moved inside (4px from left edge)
                drawPinSocket(draw_list, socketPos, m_inputPins[i].type, true);
            }
            
            // Draw output sockets on right side (inside the node)
            float outputSpacing = size.y / (m_outputPins.size() + 1);
            for (size_t i = 0; i < m_outputPins.size(); ++i) {
                ImVec2 socketPos = screenPos + ImVec2(size.x - 4.0f, outputSpacing * (i + 1)); // Moved inside (4px from right edge)
                drawPinSocket(draw_list, socketPos, m_outputPins[i].type, false);
            }
        }

        // Add this helper method to update pin positions for interaction:
        void updatePinPositions(const ImVec2& screenPos, const ImVec2& size) {
            // Update input pin positions
            float inputSpacing = size.y / (m_inputPins.size() + 1);
            for (size_t i = 0; i < m_inputPins.size(); ++i) {
                if (m_inputPins[i].pin_ptr) {
                    ImVec2 pinPos = screenPos + ImVec2(0.0f, inputSpacing * (i + 1));
                    m_inputPins[i].pin_ptr->setPos(pinPos);
                }
            }
            
            // Update output pin positions  
            float outputSpacing = size.y / (m_outputPins.size() + 1);
            for (size_t i = 0; i < m_outputPins.size(); ++i) {
                if (m_outputPins[i].pin_ptr) {
                    ImVec2 pinPos = screenPos + ImVec2(size.x, outputSpacing * (i + 1));
                    m_outputPins[i].pin_ptr->setPos(pinPos);
                }
            }
        }
        

        bool usesCustomHover() const override { return true; }
        

        bool customIsHovered() const override {
            auto handler = getHandler();
            if (!handler) return false;
            
            ImVec2 pos = getPos();
            ImVec2 size = getSize();
            ImVec2 screenMin = handler->grid2screen(pos);
            ImVec2 screenMax = screenMin + size;
            
            return ImGui::IsMouseHoveringRect(screenMin, screenMax);
        }
        

        const ImVec2& getSize() const override { 
            static ImVec2 minimalSize(80, 60); // Fixed size for minimal nodes
            return minimalSize;
        }
        

        ImVec2 getVisualSize() const override { 
            return getSize();
        }
        
        SET_NODE_TYPE_NAME(minimal_node)
        

        void serialize(AT::serializer::yaml& yaml) override {
            BaseNode::serialize(yaml);
            
            // yaml.entry("minimal_node_name", m_name)
            //     .entry("input_count", static_cast<int>(m_inputPins.size()))
            //     .entry("output_count", static_cast<int>(m_outputPins.size()));
                
            // // Serialize input types
            // std::vector<int> input_types;
            // for (const auto& pin : m_inputPins) {
            //     input_types.push_back(static_cast<int>(pin.type));
            // }
            // yaml.entry("input_types", input_types);
            
            // // Serialize output types
            // std::vector<int> output_types;
            // for (const auto& pin : m_outputPins) {
            //     output_types.push_back(static_cast<int>(pin.type));
            // }
            // yaml.entry("output_types", output_types);
        }

    private:

        struct MinimalPin {
            pin_data_type type;
            std::string name;
            std::shared_ptr<ImFlow::Pin> pin_ptr;
        };

        
        void setupPins(const std::vector<pin_data_type>& input_types, 
                    const std::vector<pin_data_type>& output_types) {
            
            // Create input pins
            for (size_t i = 0; i < input_types.size(); ++i) {
                MinimalPin pin;
                pin.type = input_types[i];
                pin.name = "in_" + std::to_string(i);
                pin.pin_ptr = createTypedPin(pin.name, pin.type, true);
                m_inputPins.push_back(pin);
            }
            
            // Create output pins
            for (size_t i = 0; i < output_types.size(); ++i) {
                MinimalPin pin;
                pin.type = output_types[i];
                pin.name = "out_" + std::to_string(i);
                pin.pin_ptr = createTypedPin(pin.name, pin.type, false);
                m_outputPins.push_back(pin);
            }
        }


        std::shared_ptr<ImFlow::Pin> createTypedPin(const std::string& name, pin_data_type type, bool is_input) {
            switch (type) {
                case pin_data_type::integer:
                    if (is_input) {
                        return addIN<int>(name, 0, ImFlow::ConnectionFilter::SameType(), createPinStyle(type));
                    } else {
                        auto outPin = addOUT<int>(name, createPinStyle(type));
                        if (m_operation) {
                            outPin->behaviour([this, name]() {
                                return computeOutput<int>(name);
                            });
                        } else {
                            outPin->behaviour([this, name]() { return 0; });
                        }
                        return outPin;
                    }
                    break;
                    
                case pin_data_type::floating_point:
                    if (is_input) {
                        return addIN<double>(name, 0.0, ImFlow::ConnectionFilter::SameType(), createPinStyle(type));
                    } else {
                        auto outPin = addOUT<double>(name, createPinStyle(type));
                        if (m_operation) {
                            outPin->behaviour([this, name]() {
                                return computeOutput<double>(name);
                            });
                        } else {
                            outPin->behaviour([this, name]() { return 0.0; });
                        }
                        return outPin;
                    }
                    break;
                    
                case pin_data_type::string:
                    if (is_input) {
                        return addIN<std::string>(name, std::string(""), ImFlow::ConnectionFilter::SameType(), createPinStyle(type));
                    } else {
                        auto outPin = addOUT<std::string>(name, createPinStyle(type));
                        if (m_operation) {
                            outPin->behaviour([this, name]() {
                                return computeOutput<std::string>(name);
                            });
                        } else {
                            outPin->behaviour([this, name]() { return std::string(""); });
                        }
                        return outPin;
                    }
                    break;
                    
                case pin_data_type::boolean:
                    if (is_input) {
                        return addIN<bool>(name, false, ImFlow::ConnectionFilter::SameType(), createPinStyle(type));
                    } else {
                        auto outPin = addOUT<bool>(name, createPinStyle(type));
                        if (m_operation) {
                            outPin->behaviour([this, name]() {
                                return computeOutput<bool>(name);
                            });
                        } else {
                            outPin->behaviour([this, name]() { return false; });
                        }
                        return outPin;
                    }
                    break;
                    
                case pin_data_type::execution:
                    // Minimal nodes don't use execution pins
                    return nullptr;
            }
            return nullptr;
        }

        template<typename T>
        T computeOutput(const std::string& pin_name) {
            if (!m_operation) return T{};
            
            // Collect input values
            std::vector<function_value> inputs;
            for (const auto& pin : m_inputPins) {
                std::visit([&](auto&& defaultValue) {
                    using U = std::decay_t<decltype(defaultValue)>;
                    inputs.push_back(getInVal<U>(pin.name));
                }, getDefaultValueForType(pin.type));
            }
            
            // Call the operation
            auto results = m_operation(inputs);
            
            // Find the corresponding output
            for (size_t i = 0; i < m_outputPins.size(); ++i) {
                if (m_outputPins[i].name == pin_name && i < results.size()) {
                    return std::get<T>(results[i]);
                }
            }
            
            return T{};
        }
        

        function_value getDefaultValueForType(pin_data_type type) {
            switch (type) {
                case pin_data_type::integer: return 0;
                case pin_data_type::floating_point: return 0.0;
                case pin_data_type::string: return std::string("");
                case pin_data_type::boolean: return false;
                case pin_data_type::execution: return Execution{};
            }
            return 0;
        }
        
            
        void drawPinSocket(ImDrawList* draw_list, const ImVec2& center, pin_data_type type, bool is_input) {
            ImU32 color = pin_data_typeToColor(type);
            float radius = 4.0f;
            
            // Draw filled circle for the socket
            draw_list->AddCircleFilled(center, radius, color);
            
            // Draw outline
            draw_list->AddCircle(center, radius, IM_COL32(255, 255, 255, 255), 0, 1.0f);
            
            // Update the actual pin connection point to match the visual socket
            // This ensures the connection point is where the socket is drawn
        }


        std::shared_ptr<ImFlow::NodeStyle> create_minimal_node_style() {
            auto style = std::make_shared<ImFlow::NodeStyle>(
                IM_COL32(0, 0, 0, 0),  // Transparent header
                ImColor(255, 255, 255, 255), 
                4.0f  // Radius for rounded corners
            );
            style->bg = IM_COL32(60, 60, 60, 255);
            style->border_thickness = 2.0f;
            style->border_selected_thickness = 3.0f;
            style->border_color = IM_COL32(100, 100, 100, 255);
            style->border_selected_color = IM_COL32(255, 255, 100, 255);
            style->padding = ImVec4(0.f, 0.f, 0.f, 0.f); // No padding
            
            return style;
        }

        std::string m_name;
        std::vector<MinimalPin> m_inputPins;
        std::vector<MinimalPin> m_outputPins;
        std::function<std::vector<function_value>(const std::vector<function_value>&)> m_operation;
    };

}
