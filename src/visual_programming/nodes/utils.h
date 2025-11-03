#pragma once

#include <ImNodeFlow.h>

namespace AT {

    // Define execution data type
    struct Execution {};

    // Supported data types for function nodes
    using function_value = std::variant<int, double, float, bool, std::string, Execution>;


    // Custom style creator functions
    static ref<ImFlow::NodeStyle> create_custom_node_style(ImU32 header_color, const char* title = "Node")
    {
        auto style = std::make_shared<ImFlow::NodeStyle>(
            header_color, 
            ImColor(250, 250, 250, 255), 
            3.0f
        );
        style->padding = ImVec4(10.f, 0.f, 10.f, 0.f);
        style->bg = ImColor(60, 60, 60, 255);
        style->border_thickness = 1.5f;
        style->border_selected_thickness = 2.5f;
        return style;
    }

    
    enum class pin_data_type : u16
    {
        integer = 0,
        floating_point,
        string,
        boolean,
        execution
        // vector,
        // unordered_map,
        // unordered_set,
        // array
    };


    inline ImU32 pin_data_typeToColor(pin_data_type type)
    {
        switch (type)
        {
            case pin_data_type::integer:        return IM_COL32(100, 150, 255, 255);  // Blue
            case pin_data_type::floating_point: return IM_COL32(100, 255, 100, 255);  // Green
            case pin_data_type::string:         return IM_COL32(255, 255, 100, 255);  // Yellow
            case pin_data_type::boolean:        return IM_COL32(255, 100, 100, 255);  // Red
            case pin_data_type::execution:      return IM_COL32(255, 255, 255, 255);  // White
            default:                          return IM_COL32(128, 128, 128, 255);  // Gray for unknown
        }
    }

    // Helper function to create pin style with color
    inline ref<ImFlow::PinStyle> createPinStyle(pin_data_type type)
    {
        return std::make_shared<ImFlow::PinStyle>(ImFlow::PinStyle(pin_data_typeToColor(type), 0, 4.f, 4.67f, 3.7f, 1.f));
    }

    // Overload for execution pins to keep their original style
    inline ref<ImFlow::PinStyle> createPinStyle()
    {
        return ImFlow::PinStyle::execution();
    }
    

    struct function_parameter
    {
        std::string name;
        pin_data_type type;
        function_value defaultValue;
        bool required = true;
        std::string description;
    };


    struct function_definition
    {
        std::string title;
        std::string description;
        std::string category;
        std::vector<function_parameter> inputs;
        std::vector<function_parameter> outputs;
        std::function<std::vector<function_value>(const std::vector<function_value>&)> function;
        bool hasExecutionFlow = false;
    };

}
