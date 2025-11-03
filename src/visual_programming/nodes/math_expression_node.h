#pragma once

#include "utils.h"
#include "exprtk.hpp"

namespace AT {

    class math_expression_node : public ImFlow::BaseNode {
    public:
        
        SET_NODE_TYPE_NAME(math_expression_node)


        math_expression_node() {
            setTitle("Math Expression");
            setStyle(create_custom_node_style(IM_COL32(71, 142, 200, 255)));
            
            // Execution pins
            addIN<Execution>("Exec In", Execution{}, ImFlow::ConnectionFilter::SameType(), ImFlow::PinStyle::execution());
            addOUT<Execution>("Exec Out", ImFlow::PinStyle::execution())->behaviour([]() { return Execution{}; });
            
            // Initialize with a default expression
            m_expression = "a + b";
            m_previousExpression = m_expression;
            strncpy(m_expressionBuffer, m_expression.c_str(), sizeof(m_expressionBuffer));
            
            // Initialize ExprTk
            m_symbolTable.add_constants();
            parseExpression();
        }
        

        void draw() override {
            ImGui::PushItemWidth(250);
            
            // Expression input
            if (ImGui::InputText("##Expression", m_expressionBuffer, sizeof(m_expressionBuffer), 
                                ImGuiInputTextFlags_EnterReturnsTrue)) {
                m_expression = m_expressionBuffer;
                if (parseExpression()) {
                    m_previousExpression = m_expression;
                } else {
                    // Revert to previous valid expression if parsing fails
                    m_expression = m_previousExpression;
                    strncpy(m_expressionBuffer, m_previousExpression.c_str(), sizeof(m_expressionBuffer));
                }
            }
            
            // Parse button
            ImGui::SameLine();
            if (ImGui::Button("Parse")) {
                m_expression = m_expressionBuffer;
                if (parseExpression()) {
                    m_previousExpression = m_expression;
                } else {
                    m_expression = m_previousExpression;
                    strncpy(m_expressionBuffer, m_previousExpression.c_str(), sizeof(m_expressionBuffer));
                }
            }
            
            // Help marker
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("ExprTk Math Expression:\n"
                                    "- Use standard math notation\n"
                                    "- Variables are automatically detected\n"
                                    "- Supports: +, -, *, /, ^, sin, cos, tan, log, exp, sqrt, etc.\n"
                                    "- Example: a + b * sin(c)\n"
                                    "- Example: sqrt(x^2 + y^2)\n"
                                    "- Example: if(x > 0, x, -x)");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            
            ImGui::PopItemWidth();
            
            // Show parsed information
            ImGui::Spacing();
            ImGui::Text("Input Variables: %zu", m_variables.size());
            
            // Update variable values from input pins before evaluation
            updateVariableValues();
            
            // Show current values if available
            if (!m_variables.empty()) {
                ImGui::Spacing();
                ImGui::Text("Current Values:");
                
                for (const auto& [name, value] : m_variables) {
                    ImGui::Text("%s: %.3f", name.c_str(), value);
                }
                
                if (m_expressionCompiled) {
                    try {
                        double result = m_expressionEvaluator.value();
                        ImGui::Text("Result: %.6f", result);
                    } catch (...) {
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Result: Evaluation Error");
                    }
                }
            }
            
            // Show error if any
            if (!m_parseError.empty()) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: %s", m_parseError.c_str());
            }
        }
                

        void serialize(AT::serializer::yaml& yaml) override {
            BaseNode::serialize(yaml);
            
            yaml.entry("expression", m_expression)
                .entry("previous_expression", m_previousExpression);
                
            if (yaml.get_option() == AT::serializer::option::load_from_file) {
                strncpy(m_expressionBuffer, m_expression.c_str(), sizeof(m_expressionBuffer));
                parseExpression();
            }
        }

    private:

        bool parseExpression() {
            m_parseError.clear();
            
            // Clear previous state
            m_variables.clear();
            m_symbolTable.clear();
            m_symbolTable.add_constants();
            
            // Extract variables from expression
            std::set<std::string> variableNames = extractVariables(m_expression);
            
            if (variableNames.empty()) {
                m_parseError = "No variables found in expression";
                return false;
            }
            
            // Create variables in symbol table
            for (const auto& varName : variableNames) {
                // Initialize variable with 0.0
                m_variables[varName] = 0.0;
                m_symbolTable.add_variable(varName, m_variables[varName]);
            }
            
            // Compile expression
            m_expressionEvaluator.register_symbol_table(m_symbolTable);
            
            exprtk::parser<double> parser;
            m_expressionCompiled = parser.compile(m_expression, m_expressionEvaluator);
            
            if (!m_expressionCompiled) {
                m_parseError = "Failed to compile expression";
                return false;
            }
            
            // Recreate pins based on parsed variables
            recreatePins();
            
            return true;
        }
        

        std::set<std::string> extractVariables(const std::string& expression) {
            std::set<std::string> variables;
            
            // Simple variable extraction - looks for valid variable names
            std::string currentVar;
            bool inVar = false;
            
            for (char c : expression) {
                if (std::isalpha(c) || c == '_') {
                    currentVar += c;
                    inVar = true;
                } else if (inVar && (std::isdigit(c) || c == '.')) {
                    // Allow digits and dots in variable names (for things like var1, my.var, etc.)
                    currentVar += c;
                } else {
                    if (inVar && !currentVar.empty()) {
                        // Check if it's not a function name or constant
                        if (!isReservedWord(currentVar)) {
                            variables.insert(currentVar);
                        }
                        currentVar.clear();
                    }
                    inVar = false;
                }
            }
            
            // Don't forget the last variable
            if (inVar && !currentVar.empty() && !isReservedWord(currentVar)) {
                variables.insert(currentVar);
            }
            
            return variables;
        }
        

        bool isReservedWord(const std::string& word) {
            // Common math functions and constants that shouldn't be treated as variables
            static const std::set<std::string> reservedWords = {
                "sin", "cos", "tan", "asin", "acos", "atan", "atan2",
                "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",
                "log", "log10", "exp", "sqrt", "abs", "ceil", "floor",
                "round", "min", "max", "clamp", "pi", "e", "true", "false",
                "if", "else", "while", "for", "repeat", "return", "var", "const"
            };
            
            return reservedWords.count(word) > 0;
        }
        

        void recreatePins() {
            // Store the names of pins to remove
            std::vector<std::string> pinsToRemove;
            
            // Collect non-execution input pins to remove
            for (const auto& pin : getIns()) {
                if (pin->getName() != "Exec In") {
                    pinsToRemove.push_back(pin->getName());
                }
            }
            
            // Remove the collected input pins
            for (const auto& pinName : pinsToRemove) {
                dropIN(pinName.c_str());
            }
            
            pinsToRemove.clear();
            
            // Collect non-execution output pins to remove
            for (const auto& pin : getOuts()) {
                if (pin->getName() != "Exec Out") {
                    pinsToRemove.push_back(pin->getName());
                }
            }
            
            // Remove the collected output pins
            for (const auto& pinName : pinsToRemove) {
                dropOUT(pinName.c_str());
            }
            
            // Create input pins for variables
            for (const auto& [varName, varValue] : m_variables) {
                addIN<double>(varName, 0.0, ImFlow::ConnectionFilter::SameType());
            }
            
            // Create output pin for the result
            auto outPin = addOUT<double>("Result");
            outPin->behaviour([this]() {
                // Update variable values before evaluation
                updateVariableValues();
                
                if (m_expressionCompiled) {
                    try {
                        return m_expressionEvaluator.value();
                    } catch (...) {
                        return 0.0;
                    }
                }
                return 0.0;
            });
        }
        

        void updateVariableValues() {
            // Update variable values from input pins
            for (auto& [varName, varValue] : m_variables) {
                varValue = getInVal<double>(varName);
            }
        }
        
        std::string m_expression;
        std::string m_previousExpression;
        std::string m_parseError;
        char m_expressionBuffer[512] = {0};
        
        // ExprTk members
        exprtk::symbol_table<double> m_symbolTable;
        exprtk::expression<double> m_expressionEvaluator;
        bool m_expressionCompiled = false;
        
        // Variable storage - maps variable names to their current values
        std::unordered_map<std::string, double> m_variables;
    };

}
