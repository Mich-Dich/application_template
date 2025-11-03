#pragma once

#include "minimal_node.h"

namespace AT {

    struct minimal_node_definition {
        std::string name;
        std::string description;
        std::string category;
        std::vector<pin_data_type> inputs;
        std::vector<pin_data_type> outputs;
        std::function<std::vector<function_value>(const std::vector<function_value>&)> operation;
    };


    class minimal_node_registry {
    public:

        static minimal_node_registry& getInstance() {
            static minimal_node_registry instance;
            return instance;
        }
        
        void registerNode(const minimal_node_definition& def) {
            m_nodes[def.name] = def;
        }
        
        const minimal_node_definition* getNode(const std::string& name) const {
            auto it = m_nodes.find(name);
            return it != m_nodes.end() ? &it->second : nullptr;
        }
        
        const std::unordered_map<std::string, minimal_node_definition>& getAllNodes() const {
            return m_nodes;
        }
        
        std::vector<std::string> getNodesByCategory(const std::string& category) const {
            std::vector<std::string> result;
            for (const auto& [name, def] : m_nodes) {
                if (def.category == category) {
                    result.push_back(name);
                }
            }
            return result;
        }

    private:

        minimal_node_registry() {
            initialize_standard_nodes();
        }
    
        
        void initialize_standard_nodes() {

            // Add node
            registerNode({
                "Add",
                "Adds two numbers",
                "Math",
                {pin_data_type::floating_point, pin_data_type::floating_point},
                {pin_data_type::floating_point},
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {a + b};
                }
            });
            
            // Multiply node
            registerNode({
                "Multiply", 
                "Multiplies two numbers",
                "Math",
                {pin_data_type::floating_point, pin_data_type::floating_point},
                {pin_data_type::floating_point},
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {a * b};
                }
            });
            
            // Subtract node
            registerNode({
                "Subtract",
                "Subtracts two numbers",
                "Math",
                {pin_data_type::floating_point, pin_data_type::floating_point},
                {pin_data_type::floating_point},
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {a - b};
                }
            });
            
            // Divide node
            registerNode({
                "Divide",
                "Divides two numbers",
                "Math",
                {pin_data_type::floating_point, pin_data_type::floating_point},
                {pin_data_type::floating_point},
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {b != 0 ? a / b : 0.0};
                }
            });

            // And node (boolean)
            registerNode({
                "And",
                "Logical AND of two boolean values",
                "Logic",
                {pin_data_type::boolean, pin_data_type::boolean},
                {pin_data_type::boolean},
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    bool a = std::get<bool>(inputs[0]);
                    bool b = std::get<bool>(inputs[1]);
                    return {a && b};
                }
            });

            // Or node (boolean)
            registerNode({
                "Or",
                "Logical OR of two boolean values",
                "Logic",
                {pin_data_type::boolean, pin_data_type::boolean},
                {pin_data_type::boolean},
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    bool a = std::get<bool>(inputs[0]);
                    bool b = std::get<bool>(inputs[1]);
                    return {a || b};
                }
            });

            // Concatenate node (string)
            registerNode({
                "Concatenate",
                "Concatenates two strings",
                "String",
                {pin_data_type::string, pin_data_type::string},
                {pin_data_type::string},
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    std::string a = std::get<std::string>(inputs[0]);
                    std::string b = std::get<std::string>(inputs[1]);
                    return {a + b};
                }
            });

            // Compare node
            registerNode({
                "Compare",
                "Compares two numbers (A > B)",
                "Logic",
                {pin_data_type::floating_point, pin_data_type::floating_point},
                {pin_data_type::boolean},
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {a > b};
                }
            });

        }


        std::unordered_map<std::string, minimal_node_definition> m_nodes;
    };

}
