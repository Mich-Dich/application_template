#pragma once

#include "function_node.h"
#include <unordered_map>

namespace AT {

    class FunctionRegistry {
    public:
        static FunctionRegistry& getInstance() {
            static FunctionRegistry instance;
            return instance;
        }
        
        void registerFunction(const FunctionDefinition& def) {
            m_functions[def.title] = def;
        }
        
        const FunctionDefinition* getFunction(const std::string& title) const {
            auto it = m_functions.find(title);
            return it != m_functions.end() ? &it->second : nullptr;
        }
        
        const std::unordered_map<std::string, FunctionDefinition>& getAllFunctions() const {
            return m_functions;
        }
        
        std::vector<std::string> getFunctionsByCategory(const std::string& category) const {
            std::vector<std::string> result;
            for (const auto& [name, def] : m_functions) {
                if (def.category == category) {
                    result.push_back(name);
                }
            }
            return result;
        }

    private:

        FunctionRegistry() {
            initializeStandardFunctions();
        }
    
        
        void initializeStandardFunctions() {
            // Math functions
            registerFunction({
                "Add",
                "Adds two numbers",
                "Math",
                {
                    {"a", "double", 0.0, true, "First number"},
                    {"b", "double", 0.0, true, "Second number"}
                },
                {
                    {"result", "double", 0.0, true, "Sum of a and b"}
                },
                [](const std::vector<FunctionValue>& inputs) -> std::vector<FunctionValue> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {a + b};
                },
                false
            });
            
            registerFunction({
                "Multiply", 
                "Multiplies two numbers",
                "Math",
                {
                    {"a", "double", 0.0, true, "First number"},
                    {"b", "double", 0.0, true, "Second number"}
                },
                {
                    {"result", "double", 0.0, true, "Product of a and b"}
                },
                [](const std::vector<FunctionValue>& inputs) -> std::vector<FunctionValue> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {a * b};
                },
                false
            });
            
            registerFunction({
                "Square Root",
                "Calculates square root of a number",
                "Math", 
                {
                    {"x", "double", 0.0, true, "Input value"}
                },
                {
                    {"result", "double", 0.0, true, "Square root of x"}
                },
                [](const std::vector<FunctionValue>& inputs) -> std::vector<FunctionValue> {
                    double x = std::get<double>(inputs[0]);
                    return {std::sqrt(x)};
                },
                false
            });
            
            // String functions
            registerFunction({
                "Concatenate Strings",
                "Joins two strings together",
                "String",
                {
                    {"str1", "string", std::string("Hello"), true, "First string"},
                    {"str2", "string", std::string("World"), true, "Second string"}
                },
                {
                    {"result", "string", std::string(""), true, "Concatenated string"}
                },
                [](const std::vector<FunctionValue>& inputs) -> std::vector<FunctionValue> {
                    std::string a = std::get<std::string>(inputs[0]);
                    std::string b = std::get<std::string>(inputs[1]);
                    return {a + b};
                },
                false
            });
            
            // Logic functions
            registerFunction({
                "Compare Numbers",
                "Compares two numbers",
                "Logic",
                {
                    {"a", "double", 0.0, true, "First number"},
                    {"b", "double", 0.0, true, "Second number"}
                },
                {
                    {"a_equals_b", "bool", false, true, "True if a equals b"},
                    {"a_greater_b", "bool", false, true, "True if a greater than b"},
                    {"a_less_b", "bool", false, true, "True if a less than b"}
                },
                [](const std::vector<FunctionValue>& inputs) -> std::vector<FunctionValue> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {a == b, a > b, a < b};
                },
                false
            });
        }


        std::unordered_map<std::string, FunctionDefinition> m_functions;
    };

}
