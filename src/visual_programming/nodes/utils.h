#pragma once

#include <ImNodeFlow.h>

namespace AT {

    // Define execution data type
    struct Execution {};

    // Supported data types for function nodes
    using function_value = std::variant<
        int8, int16, int32, int64,
        u8, u16, u32, u64,
        f32, f64,
        std::string, bool, Execution
    >;


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
        integer_8 = 0,
        integer_16,
        integer_32,
        integer_64,

        uinteger_8,
        uinteger_16,
        uinteger_32,
        uinteger_64,
        
        float_32,
        float_64,
        
        string,
        boolean,
        execution
        // vector,
        // unordered_map,
        // unordered_set,
        // array
    };


    // Type mapping traits
    template<pin_data_type Type>
    struct pin_type_traits;

    // Specializations for each pin type
    template<> struct pin_type_traits<pin_data_type::integer_8>    { using type = int8_t;  static constexpr type default_value = 0; };
    template<> struct pin_type_traits<pin_data_type::integer_16>   { using type = int16_t; static constexpr type default_value = 0; };
    template<> struct pin_type_traits<pin_data_type::integer_32>   { using type = int32_t; static constexpr type default_value = 0; };
    template<> struct pin_type_traits<pin_data_type::integer_64>   { using type = int64_t; static constexpr type default_value = 0; };

    template<> struct pin_type_traits<pin_data_type::uinteger_8>   { using type = uint8_t;  static constexpr type default_value = 0; };
    template<> struct pin_type_traits<pin_data_type::uinteger_16>  { using type = uint16_t; static constexpr type default_value = 0; };
    template<> struct pin_type_traits<pin_data_type::uinteger_32>  { using type = uint32_t; static constexpr type default_value = 0; };
    template<> struct pin_type_traits<pin_data_type::uinteger_64>  { using type = uint64_t; static constexpr type default_value = 0; };

    template<> struct pin_type_traits<pin_data_type::float_32>     { using type = float;   static constexpr type default_value = 0.0f; };
    template<> struct pin_type_traits<pin_data_type::float_64>     { using type = double;  static constexpr type default_value = 0.0; };

    template<> struct pin_type_traits<pin_data_type::string>       { using type = std::string; static constexpr auto default_value = ""; };
    template<> struct pin_type_traits<pin_data_type::boolean>      { using type = bool;    static constexpr type default_value = false; };
    template<> struct pin_type_traits<pin_data_type::execution>    { using type = Execution; };


    // Helper to get the type for a pin_data_type
    template<pin_data_type Type>
    using pin_type_t = typename pin_type_traits<Type>::type;


    // Helper to get default value for a pin_data_type
    template<pin_data_type Type>
    constexpr auto pin_default_value = pin_type_traits<Type>::default_value;


    inline ImU32 pin_data_type_to_color(pin_data_type type) {
        switch (type) {

            // Signed integers - shades of blue
            case pin_data_type::integer_8:   return IM_COL32(80, 120, 255, 255);   // Light blue
            case pin_data_type::integer_16:  return IM_COL32(60, 100, 255, 255);   // Medium blue
            case pin_data_type::integer_32:  return IM_COL32(40, 80, 255, 255);    // Blue
            case pin_data_type::integer_64:  return IM_COL32(20, 60, 255, 255);    // Dark blue

            // Unsigned integers - shades of green
            case pin_data_type::uinteger_8:  return IM_COL32(80, 255, 120, 255);   // Light green
            case pin_data_type::uinteger_16: return IM_COL32(60, 255, 100, 255);   // Medium green
            case pin_data_type::uinteger_32: return IM_COL32(40, 255, 80, 255);    // Green
            case pin_data_type::uinteger_64: return IM_COL32(20, 255, 60, 255);    // Dark green

            // Floating point - shades of purple
            case pin_data_type::float_32:    return IM_COL32(180, 100, 255, 255);  // Light purple
            case pin_data_type::float_64:    return IM_COL32(160, 80, 255, 255);   // Purple

            // Other types
            case pin_data_type::string:      return IM_COL32(255, 255, 100, 255);  // Yellow
            case pin_data_type::boolean:     return IM_COL32(255, 100, 100, 255);  // Red
            case pin_data_type::execution:   return IM_COL32(255, 255, 255, 255);  // White
            
            default:                         return IM_COL32(128, 128, 128, 255);  // Gray for unknown
        }
    }


    function_value get_default_value_for_type(pin_data_type type) {
        switch (type) {
            case pin_data_type::integer_8:    return pin_default_value<pin_data_type::integer_8>;
            case pin_data_type::integer_16:   return pin_default_value<pin_data_type::integer_16>;
            case pin_data_type::integer_32:   return pin_default_value<pin_data_type::integer_32>;
            case pin_data_type::integer_64:   return pin_default_value<pin_data_type::integer_64>;
            
            case pin_data_type::uinteger_8:   return pin_default_value<pin_data_type::uinteger_8>;
            case pin_data_type::uinteger_16:  return pin_default_value<pin_data_type::uinteger_16>;
            case pin_data_type::uinteger_32:  return pin_default_value<pin_data_type::uinteger_32>;
            case pin_data_type::uinteger_64:  return pin_default_value<pin_data_type::uinteger_64>;
            
            case pin_data_type::float_32:     return pin_default_value<pin_data_type::float_32>;
            case pin_data_type::float_64:     return pin_default_value<pin_data_type::float_64>;
            
            case pin_data_type::string:       return pin_default_value<pin_data_type::string>;
            case pin_data_type::boolean:      return pin_default_value<pin_data_type::boolean>;
            case pin_data_type::execution:    return Execution{};
        }
        return 0;
    }


    // Helper function to create pin style with color
    inline ref<ImFlow::PinStyle> createPinStyle(pin_data_type type)
    {
        return std::make_shared<ImFlow::PinStyle>(ImFlow::PinStyle(pin_data_type_to_color(type), 0, 4.f, 4.67f, 3.7f, 1.f));
    }


    // Overload for execution pins to keep their original style
    inline ref<ImFlow::PinStyle> createPinStyle()       { return ImFlow::PinStyle::execution(); }
    

    struct function_parameter {

        std::string                                                                     name;
        pin_data_type                                                                   type;
        function_value                                                                  defaultValue;
        bool                                                                            required = true;
        std::string                                                                     description;
    };


    struct function_definition {

        std::string                                                                     title;
        std::string                                                                     description;
        std::string                                                                     category;
        std::vector<function_parameter>                                                 inputs;
        std::vector<function_parameter>                                                 outputs;
        std::function<std::vector<function_value>(const std::vector<function_value>&)>  function;
        bool                                                                            hasExecutionFlow = false;
    };

}
