#pragma once

#include <unordered_map>
#include "util/util.h"
#include "function_node.h"


namespace AT {

    class function_registry {
    public:

        static function_registry& getInstance() {
            static function_registry instance;
            return instance;
        }
        
        void registerFunction(const function_definition& def) {
            m_functions[def.title] = def;
        }
        
        const function_definition* getFunction(const std::string& title) const {
            auto it = m_functions.find(title);
            return it != m_functions.end() ? &it->second : nullptr;
        }
        
        const std::unordered_map<std::string, function_definition>& getAllFunctions() const {
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

        function_registry() {
            initialize_standard_functions();
        }
    
        
        void initialize_standard_functions() {

            // Add
            registerFunction({
                "Add",
                "Adds two numbers",
                "Math",
                {
                    {"a", pin_data_type::float_32, 0.0, true, "First number"},
                    {"b", pin_data_type::float_32, 0.0, true, "Second number"}
                },
                {
                    {"result", pin_data_type::float_32, 0.0, true, "Sum of a and b"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {a + b};
                },
                false
            });
            
            // Multiply
            registerFunction({
                "Multiply", 
                "Multiplies two numbers",
                "Math",
                {
                    {"a", pin_data_type::float_32, 0.0, true, "First number"},
                    {"b", pin_data_type::float_32, 0.0, true, "Second number"}
                },
                {
                    {"result", pin_data_type::float_32, 0.0, true, "Product of a and b"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {a * b};
                },
                false
            });
            
            // Square Root
            registerFunction({
                "Square Root",
                "Calculates square root of a number",
                "Math", 
                {
                    {"x", pin_data_type::float_32, 0.0, true, "Input value"}
                },
                {
                    {"result", pin_data_type::float_32, 0.0, true, "Square root of x"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double x = std::get<double>(inputs[0]);
                    return {std::sqrt(x)};
                },
                false
            });
            
            // Concatenate Strings
            registerFunction({
                "Concatenate Strings",
                "Joins two strings together",
                "String",
                {
                    {"str1", pin_data_type::string, std::string("Hello"), true, "First string"},
                    {"str2", pin_data_type::string, std::string("World"), true, "Second string"}
                },
                {
                    {"result", pin_data_type::string, std::string(""), true, "Concatenated string"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    std::string a = std::get<std::string>(inputs[0]);
                    std::string b = std::get<std::string>(inputs[1]);
                    return {a + b};
                },
                false
            });
            
            // Compare Numbers
            registerFunction({
                "Compare Numbers",
                "Compares two numbers",
                "Logic",
                {
                    {"a", pin_data_type::float_32, 0.0, true, "First number"},
                    {"b", pin_data_type::float_32, 0.0, true, "Second number"}
                },
                {
                    {"a_equals_b", pin_data_type::boolean, false, true, "True if a equals b"},
                    {"a_greater_b", pin_data_type::boolean, false, true, "True if a greater than b"},
                    {"a_less_b", pin_data_type::boolean, false, true, "True if a less than b"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {a == b, a > b, a < b};
                },
                false
            });

            // Random Integer
            registerFunction({
                "Random Integer",
                "Generates a random integer within specified range",
                "Random",
                {
                    {"min", pin_data_type::integer_32, 0, true, "Minimum value (inclusive)"},
                    {"max", pin_data_type::integer_32, 100, true, "Maximum value (inclusive)"}
                },
                {
                    {"result", pin_data_type::integer_32, 0, true, "Random integer"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    int min = std::get<int>(inputs[0]);
                    int max = std::get<int>(inputs[1]);
                    AT::util::random rng;
                    return {rng.get<int>(min, max)};
                },
                false
            });

            // Random Float
            registerFunction({
                "Random Float",
                "Generates a random floating-point number within specified range",
                "Random",
                {
                    {"min", pin_data_type::float_32, 0.0, true, "Minimum value (inclusive)"},
                    {"max", pin_data_type::float_32, 1.0, true, "Maximum value (inclusive)"}
                },
                {
                    {"result", pin_data_type::float_32, 0.0, true, "Random float"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double min = std::get<double>(inputs[0]);
                    double max = std::get<double>(inputs[1]);
                    AT::util::random rng;
                    return {rng.get<double>(min, max)};
                },
                false
            });

            // Random Boolean
            registerFunction({
                "Random Boolean",
                "Generates a random boolean value",
                "Random",
                {
                    {"probability", pin_data_type::float_32, 0.5, true, "Probability of true (0.0 to 1.0)"}
                },
                {
                    {"result", pin_data_type::boolean, false, true, "Random boolean"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double probability = std::get<double>(inputs[0]);
                    AT::util::random rng;
                    return {rng.get_percent(static_cast<float>(probability))};
                },
                false
            });

            registerFunction({
                "Random String",
                "Generates a random alphanumeric string",
                "Random",
                {
                    {"length", pin_data_type::integer_32, 10, true, "Length of the string"}
                },
                {
                    {"result", pin_data_type::string, std::string(""), true, "Random string"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    int length = std::get<int>(inputs[0]);
                    AT::util::random rng;
                    return {rng.get_string(static_cast<size_t>(length))};
                },
                false
            });

            // Math constants functions
            registerFunction({
                "Get Pi",
                "Returns the mathematical constant π (pi)",
                "Constants",
                {},
                {
                    {"pi", pin_data_type::float_32, 0.0, true, "Value of π"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    return {AT::pi<double>()};
                },
                false
            });

            registerFunction({
                "Get Euler's Number",
                "Returns the mathematical constant e (Euler's number)",
                "Constants",
                {},
                {
                    {"e", pin_data_type::float_32, 0.0, true, "Value of e"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    return {AT::e<double>()};
                },
                false
            });

            registerFunction({
                "Get Golden Ratio",
                "Returns the golden ratio constant φ (phi)",
                "Constants",
                {},
                {
                    {"golden_ratio", pin_data_type::float_32, 0.0, true, "Value of golden ratio"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    return {AT::golden_ratio<double>()};
                },
                false
            });

            // String manipulation functions
            registerFunction({
                "String Length",
                "Returns the length of a string",
                "String",
                {
                    {"input", pin_data_type::string, std::string(""), true, "Input string"}
                },
                {
                    {"length", pin_data_type::integer_32, 0, true, "Length of the string"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    std::string str = std::get<std::string>(inputs[0]);
                    return {static_cast<int>(str.length())};
                },
                false
            });

            registerFunction({
                "To Upper Case",
                "Converts a string to uppercase",
                "String",
                {
                    {"input", pin_data_type::string, std::string(""), true, "Input string"}
                },
                {
                    {"result", pin_data_type::string, std::string(""), true, "Uppercase string"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    std::string str = std::get<std::string>(inputs[0]);
                    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
                    return {str};
                },
                false
            });

            registerFunction({
                "To Lower Case",
                "Converts a string to lowercase",
                "String",
                {
                    {"input", pin_data_type::string, std::string(""), true, "Input string"}
                },
                {
                    {"result", pin_data_type::string, std::string(""), true, "Lowercase string"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    std::string str = std::get<std::string>(inputs[0]);
                    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
                    return {str};
                },
                false
            });

            // Boolean logic functions
            registerFunction({
                "Logical AND",
                "Performs logical AND operation on two boolean values",
                "Logic",
                {
                    {"a", pin_data_type::boolean, false, true, "First boolean"},
                    {"b", pin_data_type::boolean, false, true, "Second boolean"}
                },
                {
                    {"result", pin_data_type::boolean, false, true, "a AND b"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    bool a = std::get<bool>(inputs[0]);
                    bool b = std::get<bool>(inputs[1]);
                    return {a && b};
                },
                false
            });

            registerFunction({
                "Logical OR",
                "Performs logical OR operation on two boolean values",
                "Logic",
                {
                    {"a", pin_data_type::boolean, false, true, "First boolean"},
                    {"b", pin_data_type::boolean, false, true, "Second boolean"}
                },
                {
                    {"result", pin_data_type::boolean, false, true, "a OR b"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    bool a = std::get<bool>(inputs[0]);
                    bool b = std::get<bool>(inputs[1]);
                    return {a || b};
                },
                false
            });

            registerFunction({
                "Logical NOT",
                "Performs logical NOT operation on a boolean value",
                "Logic",
                {
                    {"input", pin_data_type::boolean, false, true, "Input boolean"}
                },
                {
                    {"result", pin_data_type::boolean, false, true, "NOT input"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    bool input = std::get<bool>(inputs[0]);
                    return {!input};
                },
                false
            });

            // Math functions with multiple outputs
            registerFunction({
                "Min Max",
                "Finds minimum and maximum of two numbers",
                "Math",
                {
                    {"a", pin_data_type::float_32, 0.0, true, "First number"},
                    {"b", pin_data_type::float_32, 0.0, true, "Second number"}
                },
                {
                    {"min", pin_data_type::float_32, 0.0, true, "Minimum value"},
                    {"max", pin_data_type::float_32, 0.0, true, "Maximum value"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double a = std::get<double>(inputs[0]);
                    double b = std::get<double>(inputs[1]);
                    return {std::min(a, b), std::max(a, b)};
                },
                false
            });

            registerFunction({
                "Power",
                "Raises a number to a power",
                "Math",
                {
                    {"base", pin_data_type::float_32, 2.0, true, "Base number"},
                    {"exponent", pin_data_type::float_32, 3.0, true, "Exponent"}
                },
                {
                    {"result", pin_data_type::float_32, 0.0, true, "base raised to exponent"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double base = std::get<double>(inputs[0]);
                    double exponent = std::get<double>(inputs[1]);
                    return {std::pow(base, exponent)};
                },
                false
            });

            // Type conversion functions
            registerFunction({
                "Float to Integer",
                "Converts a floating-point number to integer (truncates)",
                "Conversion",
                {
                    {"input", pin_data_type::float_32, 0.0, true, "Input float"}
                },
                {
                    {"result", pin_data_type::integer_32, 0, true, "Truncated integer"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double input = std::get<double>(inputs[0]);
                    return {static_cast<int>(input)};
                },
                false
            });

            registerFunction({
                "Integer to Float",
                "Converts an integer to floating-point number",
                "Conversion",
                {
                    {"input", pin_data_type::integer_32, 0, true, "Input integer"}
                },
                {
                    {"result", pin_data_type::float_32, 0.0, true, "Converted float"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    int input = std::get<int>(inputs[0]);
                    return {static_cast<double>(input)};
                },
                false
            });

            registerFunction({
                "Boolean to String",
                "Converts a boolean value to string representation",
                "Conversion",
                {
                    {"input", pin_data_type::boolean, false, true, "Input boolean"}
                },
                {
                    {"result", pin_data_type::string, std::string(""), true, "String representation"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    bool input = std::get<bool>(inputs[0]);
                    return {input ? "true" : "false"};
                },
                false
            });

            // // Conditional execution function (uses execution pins)
            // registerFunction({
            //     "Conditional Branch",
            //     "Executes one of two output paths based on a condition",
            //     "Control Flow",
            //     {
            //         {"Exec In", pin_data_type::execution, Execution{}, true, "Execution input"},
            //         {"condition", pin_data_type::boolean, false, true, "Condition to evaluate"}
            //     },
            //     {
            //         {"True", pin_data_type::execution, Execution{}, true, "Executed if condition is true"},
            //         {"False", pin_data_type::execution, Execution{}, true, "Executed if condition is false"}
            //     },
            //     [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
            //         // For execution pins, we just return empty values since the flow is handled by the framework
            //         return {};
            //     },
            //     true  // This function has execution flow
            // });

            // Random vector generation (using your random::get_vec3)
            registerFunction({
                "Random Vector3",
                "Generates a random 3D vector within specified range",
                "Random",
                {
                    {"min", pin_data_type::float_32, -1.0, true, "Minimum value for each component"},
                    {"max", pin_data_type::float_32, 1.0, true, "Maximum value for each component"}
                },
                {
                    {"x", pin_data_type::float_32, 0.0, true, "X component"},
                    {"y", pin_data_type::float_32, 0.0, true, "Y component"},
                    {"z", pin_data_type::float_32, 0.0, true, "Z component"}
                },
                [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                    double min = std::get<double>(inputs[0]);
                    double max = std::get<double>(inputs[1]);
                    AT::util::random rng;
                    glm::vec3 vec = rng.get_vec3(static_cast<float>(min), static_cast<float>(max));
                    return {static_cast<double>(vec.x), static_cast<double>(vec.y), static_cast<double>(vec.z)};
                },
                false
            });

        }


        std::unordered_map<std::string, function_definition> m_functions;
    };

}
