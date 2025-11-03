#pragma once

#include "util/pch.h"
#include "utils.h"


namespace AT
{


    class function_node : public ImFlow::BaseNode {
    public:

        function_node(const function_definition& def) : m_definition(def)
        {
            setTitle(def.title);
            setStyle(create_custom_node_style(IM_COL32(100, 150, 200, 255)));
            
            setupPins();
        }
        

        void draw() override
        {
            ImGui::PushItemWidth(120);
            
            // Show function description as tooltip
            if (ImGui::IsItemHovered() && !m_definition.description.empty()) 
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(m_definition.description.c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            
            // Show current input values
            ImGui::Text("Inputs:");
            for (size_t i = 0; i < m_definition.inputs.size(); ++i)
            {
                const auto& input = m_definition.inputs[i];
                if (input.type == pin_data_type::execution)
                {
                    continue;
                }

                std::visit([&](auto&& value)
                {
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<T, int>)
                    {
                        ImGui::Text("%s: %d", input.name.c_str(), getInVal<T>(input.name));
                    }
                    else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>)
                    {
                        ImGui::Text("%s: %.3f", input.name.c_str(), getInVal<T>(input.name));
                    }
                    else if constexpr (std::is_same_v<T, bool>)
                    {
                        ImGui::Text("%s: %s", input.name.c_str(), getInVal<T>(input.name) ? "true" : "false");
                    }
                    else if constexpr (std::is_same_v<T, std::string>)
                    {
                        ImGui::Text("%s: %s", input.name.c_str(), getInVal<T>(input.name).c_str());
                    }
                }, input.defaultValue);
            }
            
            // Show current output values if computed
            if (!m_lastOutputs.empty()) {
                ImGui::Spacing();
                ImGui::Text("Outputs:");
                for (size_t i = 0; i < m_definition.outputs.size(); ++i) {
                    const auto& output = m_definition.outputs[i];
                    if (i < m_lastOutputs.size()) {
                        std::visit([&](auto&& value) {
                            using T = std::decay_t<decltype(value)>;
                            if constexpr (std::is_same_v<T, int>) {
                                ImGui::Text("%s: %d", output.name.c_str(), std::get<T>(m_lastOutputs[i]));
                            } else if constexpr (std::is_same_v<T, double>) {
                                ImGui::Text("%s: %.3f", output.name.c_str(), std::get<T>(m_lastOutputs[i]));
                            } else if constexpr (std::is_same_v<T, float>) {
                                ImGui::Text("%s: %.3f", output.name.c_str(), std::get<T>(m_lastOutputs[i]));
                            } else if constexpr (std::is_same_v<T, bool>) {
                                ImGui::Text("%s: %s", output.name.c_str(), std::get<T>(m_lastOutputs[i]) ? "true" : "false");
                            } else if constexpr (std::is_same_v<T, std::string>) {
                                ImGui::Text("%s: %s", output.name.c_str(), std::get<T>(m_lastOutputs[i]).c_str());
                            }
                        }, output.defaultValue);
                    }
                }
            }
            
            ImGui::PopItemWidth();
        }
        

        SET_NODE_TYPE_NAME(function_node)
        

        void serialize(AT::serializer::yaml& yaml) override {
            BaseNode::serialize(yaml);
            yaml.entry("function_title", m_definition.title);
            
            if (yaml.get_option() == AT::serializer::option::load_from_file) {
                // Look up the function definition by title
                // This would need to be implemented based on your function registry
            }
        }

    private:

        void setupPins()
        {
            // Create execution pins if needed
            if (m_definition.hasExecutionFlow)
            {
                auto execStyle = createPinStyle();
                addIN<Execution>("Exec In", Execution{}, ImFlow::ConnectionFilter::SameType(), execStyle);
                
                auto execOutStyle = createPinStyle();
                addOUT<Execution>("Exec Out", execOutStyle)->behaviour([]() { return Execution{}; });
            }
            
            // Create input pins with colored styles
            for (const auto& input : m_definition.inputs)
            {
                if (input.type == pin_data_type::execution)
                {
                    auto style = createPinStyle();
                    addIN<Execution>(input.name, Execution{}, ImFlow::ConnectionFilter::SameType(), style);
                }
                else
                {
                    std::visit([&](auto&& defaultValue)
                    {
                        using T = std::decay_t<decltype(defaultValue)>;
                        auto style = createPinStyle(input.type);
                        addIN<T>(input.name, defaultValue, ImFlow::ConnectionFilter::SameType(), style);
                    }, input.defaultValue);
                }
            }
            
            // Create output pins with colored styles
            for (const auto& output : m_definition.outputs)
            {
                if (output.type == pin_data_type::execution)
                {
                    auto style = createPinStyle();
                    addOUT<Execution>(output.name, style)->behaviour([]() { return Execution{}; });
                }
                else
                {
                    std::visit([&, outputName = output.name](auto&& defaultValue)
                    {
                        using T = std::decay_t<decltype(defaultValue)>;
                        auto style = createPinStyle(output.type);
                        auto outPin = addOUT<T>(outputName, style);
                        outPin->behaviour([this, outputName, defaultValue]() {
                            computeFunction();
                            for (size_t i = 0; i < m_definition.outputs.size(); ++i)
                            {
                                if (m_definition.outputs[i].name == outputName && i < m_lastOutputs.size())
                                {
                                    return std::get<T>(m_lastOutputs[i]);
                                }
                            }
                            return defaultValue;
                        });
                    }, output.defaultValue);
                }
            }
        }
        

        void computeFunction() {
            // Collect input values
            std::vector<function_value> inputs;
            for (const auto& input : m_definition.inputs)
            {
                if (input.type == pin_data_type::execution)
                {
                    continue;
                }

                std::visit([&](auto&& defaultValue)
                {
                    using T = std::decay_t<decltype(defaultValue)>;
                    inputs.push_back(getInVal<T>(input.name));
                }
                , input.defaultValue);
            }
            
            // Call the function
            m_lastOutputs = m_definition.function(inputs);
        }
        
        function_definition m_definition;
        std::vector<function_value> m_lastOutputs;
    };

}
