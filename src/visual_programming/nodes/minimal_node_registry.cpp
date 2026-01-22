#include "util/pch.h"
#include "minimal_node_registry.h"

namespace AT
{
    /* CONSTANTS **************************************************************************************/

    /* MACROS *****************************************************************************************/

    /* TYPES ******************************************************************************************/

    /* STATIC VARIABLES *******************************************************************************/

    /* FUNCTION IMPLEMENTATION ************************************************************************/

    /* CLASS IMPLEMENTATION ***************************************************************************/

    void minimal_node_registry::register_node(const minimal_node_definition &def)   { m_nodes[def.name] = def; }


    const minimal_node_definition *minimal_node_registry::get_node(const std::string &name) const {

        auto it = m_nodes.find(name);
        return it != m_nodes.end() ? &it->second : nullptr;
    }


    const std::unordered_map<std::string, minimal_node_definition> &minimal_node_registry::get_all_nodes() const { return m_nodes; }


    std::vector<std::string> minimal_node_registry::get_nodes_by_category(const std::string &category) const {
        std::vector<std::string> result;
        for (const auto &[name, def] : m_nodes)
            if (def.category == category)
                result.push_back(name);

        return result;
    }

    // ============================================================================================= */
    // PRIVATE */
    // ============================================================================================= */

    // Singleton constructor (private)
    minimal_node_registry::minimal_node_registry()  { initialize_standard_nodes(); }


    void minimal_node_registry::initialize_standard_nodes() {

        #define SAME_DISPLAY_NAME(name) name, name

        // Concatenate node (string)
        register_node({SAME_DISPLAY_NAME("Concatenate"),
            "Concatenates two strings",
            "String",
            {pin_data_type::string, pin_data_type::string},
            {pin_data_type::string},
            [](const std::vector<function_value>& inputs) -> std::vector<function_value>
            {
                std::string a = std::get<std::string>(inputs[0]);
                std::string b = std::get<std::string>(inputs[1]);
                return {a + b};
            }});

        // Math functions with multiple outputs
        #define MIN_MAX_NODE(name, func, pin_type, type)                                    \
            register_node({name,                                                            \
            #func,                                                                          \
            "Finds minimum of two numbers",                                                 \
            "Math",                                                                         \
            {pin_data_type::pin_type, pin_data_type::pin_type},                             \
            {pin_data_type::pin_type},                                                      \
            [](const std::vector<function_value>& inputs) -> std::vector<function_value>    \
            {                                                                               \
                const type a = std::get<type>(inputs[0]);                                   \
                const type b = std::get<type>(inputs[1]);                                   \
                return std::vector<function_value>(AT::math::func<type>(a, b));                   \
            }})

        MIN_MAX_NODE("Max Float", max, float_32, f64);
        MIN_MAX_NODE("Max Double", max, float_64, f64);
        MIN_MAX_NODE("Max Int 8", max, integer_8, int8);
        MIN_MAX_NODE("Max Int 16", max, integer_16, int16);
        MIN_MAX_NODE("Max Int 32", max, integer_32, int32);

        MIN_MAX_NODE("Max Int 64", max, integer_64, int64);
        MIN_MAX_NODE("Max Uint 8", max, uinteger_8, u8);
        MIN_MAX_NODE("Max Uint 16", max, uinteger_16, u16);
        MIN_MAX_NODE("Max Uint 32", max, uinteger_32, u32);
        MIN_MAX_NODE("Max Uint 64", max, uinteger_64, u64);

        MIN_MAX_NODE("Min Float", min, float_32, f64);
        MIN_MAX_NODE("Min Double", min, float_64, f64);
        MIN_MAX_NODE("Min Int 8", min, integer_8, int8);
        MIN_MAX_NODE("Min Int 16", min, integer_16, int16);
        MIN_MAX_NODE("Min Int 32", min, integer_32, int32);
        MIN_MAX_NODE("Min Int 64", min, integer_64, int64);
        MIN_MAX_NODE("Min Uint 8", min, uinteger_8, u8);
        MIN_MAX_NODE("Min Uint 16", min, uinteger_16, u16);
        MIN_MAX_NODE("Min Uint 32", min, uinteger_32, u32);
        MIN_MAX_NODE("Min Uint 64", min, uinteger_64, u64);

        #undef MIN_MAX_NODE

        // **************************************************************************************************/
        // TYPE CONVERSION */
        // **************************************************************************************************/

        
        // Math functions with multiple outputs
        #define NUMERIC_CONVERTER(name, in_pin_type, out_pin_type, in_type, out_type)       \
            register_node({"Max Float",                                                     \
            "",                                                                             \
            "Convert a value",                                                              \
            "Conversion",                                                                   \
            {pin_data_type::in_pin_type},                                                   \
            {pin_data_type::out_pin_type},                                                  \
            [](const std::vector<function_value>& inputs) -> std::vector<function_value>    \
            {                                                                               \
                const in_type a = std::get<in_type>(inputs[0]);                             \
                return std::vector<function_value>(static_cast<out_type>(a));               \
            }})

        // Register all numeric converters
        NUMERIC_CONVERTER("Int8 to Int16", integer_8, integer_16, int8, int16);
        NUMERIC_CONVERTER("Int8 to Int32", integer_8, integer_32, int8, int32);
        NUMERIC_CONVERTER("Int8 to Int64", integer_8, integer_64, int8, int64);
        NUMERIC_CONVERTER("Int8 to Uint8", integer_8, uinteger_8, int8, u8);
        NUMERIC_CONVERTER("Int8 to Uint16", integer_8, uinteger_16, int8, u16);
        NUMERIC_CONVERTER("Int8 to Uint32", integer_8, uinteger_32, int8, u32);
        NUMERIC_CONVERTER("Int8 to Uint64", integer_8, uinteger_64, int8, u64);
        NUMERIC_CONVERTER("Int8 to Float32", integer_8, float_32, int8, f32);
        NUMERIC_CONVERTER("Int8 to Float64", integer_8, float_64, int8, f64);

        NUMERIC_CONVERTER("Int16 to Int8", integer_16, integer_8, int16, int8);
        NUMERIC_CONVERTER("Int16 to Int32", integer_16, integer_32, int16, int32);
        NUMERIC_CONVERTER("Int16 to Int64", integer_16, integer_64, int16, int64);
        NUMERIC_CONVERTER("Int16 to Uint8", integer_16, uinteger_8, int16, u8);
        NUMERIC_CONVERTER("Int16 to Uint16", integer_16, uinteger_16, int16, u16);
        NUMERIC_CONVERTER("Int16 to Uint32", integer_16, uinteger_32, int16, u32);
        NUMERIC_CONVERTER("Int16 to Uint64", integer_16, uinteger_64, int16, u64);
        NUMERIC_CONVERTER("Int16 to Float32", integer_16, float_32, int16, f32);
        NUMERIC_CONVERTER("Int16 to Float64", integer_16, float_64, int16, f64);

        NUMERIC_CONVERTER("Int32 to Int8", integer_32, integer_8, int32, int8);
        NUMERIC_CONVERTER("Int32 to Int16", integer_32, integer_16, int32, int16);
        NUMERIC_CONVERTER("Int32 to Int64", integer_32, integer_64, int32, int64);
        NUMERIC_CONVERTER("Int32 to Uint8", integer_32, uinteger_8, int32, u8);
        NUMERIC_CONVERTER("Int32 to Uint16", integer_32, uinteger_16, int32, u16);
        NUMERIC_CONVERTER("Int32 to Uint32", integer_32, uinteger_32, int32, u32);
        NUMERIC_CONVERTER("Int32 to Uint64", integer_32, uinteger_64, int32, u64);
        NUMERIC_CONVERTER("Int32 to Float32", integer_32, float_32, int32, f32);
        NUMERIC_CONVERTER("Int32 to Float64", integer_32, float_64, int32, f64);

        NUMERIC_CONVERTER("Int64 to Int8", integer_64, integer_8, int64, int8);
        NUMERIC_CONVERTER("Int64 to Int16", integer_64, integer_16, int64, int16);
        NUMERIC_CONVERTER("Int64 to Int32", integer_64, integer_32, int64, int32);
        NUMERIC_CONVERTER("Int64 to Uint8", integer_64, uinteger_8, int64, u8);
        NUMERIC_CONVERTER("Int64 to Uint16", integer_64, uinteger_16, int64, u16);
        NUMERIC_CONVERTER("Int64 to Uint32", integer_64, uinteger_32, int64, u32);
        NUMERIC_CONVERTER("Int64 to Uint64", integer_64, uinteger_64, int64, u64);
        NUMERIC_CONVERTER("Int64 to Float32", integer_64, float_32, int64, f32);
        NUMERIC_CONVERTER("Int64 to Float64", integer_64, float_64, int64, f64);

        NUMERIC_CONVERTER("Uint8 to Int8", uinteger_8, integer_8, u8, int8);
        NUMERIC_CONVERTER("Uint8 to Int16", uinteger_8, integer_16, u8, int16);
        NUMERIC_CONVERTER("Uint8 to Int32", uinteger_8, integer_32, u8, int32);
        NUMERIC_CONVERTER("Uint8 to Int64", uinteger_8, integer_64, u8, int64);
        NUMERIC_CONVERTER("Uint8 to Uint16", uinteger_8, uinteger_16, u8, u16);
        NUMERIC_CONVERTER("Uint8 to Uint32", uinteger_8, uinteger_32, u8, u32);
        NUMERIC_CONVERTER("Uint8 to Uint64", uinteger_8, uinteger_64, u8, u64);
        NUMERIC_CONVERTER("Uint8 to Float32", uinteger_8, float_32, u8, f32);
        NUMERIC_CONVERTER("Uint8 to Float64", uinteger_8, float_64, u8, f64);

        NUMERIC_CONVERTER("Uint16 to Int8", uinteger_16, integer_8, u16, int8);
        NUMERIC_CONVERTER("Uint16 to Int16", uinteger_16, integer_16, u16, int16);
        NUMERIC_CONVERTER("Uint16 to Int32", uinteger_16, integer_32, u16, int32);
        NUMERIC_CONVERTER("Uint16 to Int64", uinteger_16, integer_64, u16, int64);
        NUMERIC_CONVERTER("Uint16 to Uint8", uinteger_16, uinteger_8, u16, u8);
        NUMERIC_CONVERTER("Uint16 to Uint32", uinteger_16, uinteger_32, u16, u32);
        NUMERIC_CONVERTER("Uint16 to Uint64", uinteger_16, uinteger_64, u16, u64);
        NUMERIC_CONVERTER("Uint16 to Float32", uinteger_16, float_32, u16, f32);
        NUMERIC_CONVERTER("Uint16 to Float64", uinteger_16, float_64, u16, f64);

        NUMERIC_CONVERTER("Uint32 to Int8", uinteger_32, integer_8, u32, int8);
        NUMERIC_CONVERTER("Uint32 to Int16", uinteger_32, integer_16, u32, int16);
        NUMERIC_CONVERTER("Uint32 to Int32", uinteger_32, integer_32, u32, int32);
        NUMERIC_CONVERTER("Uint32 to Int64", uinteger_32, integer_64, u32, int64);
        NUMERIC_CONVERTER("Uint32 to Uint8", uinteger_32, uinteger_8, u32, u8);
        NUMERIC_CONVERTER("Uint32 to Uint16", uinteger_32, uinteger_16, u32, u16);
        NUMERIC_CONVERTER("Uint32 to Uint64", uinteger_32, uinteger_64, u32, u64);
        NUMERIC_CONVERTER("Uint32 to Float32", uinteger_32, float_32, u32, f32);
        NUMERIC_CONVERTER("Uint32 to Float64", uinteger_32, float_64, u32, f64);

        NUMERIC_CONVERTER("Uint64 to Int8", uinteger_64, integer_8, u64, int8);
        NUMERIC_CONVERTER("Uint64 to Int16", uinteger_64, integer_16, u64, int16);
        NUMERIC_CONVERTER("Uint64 to Int32", uinteger_64, integer_32, u64, int32);
        NUMERIC_CONVERTER("Uint64 to Int64", uinteger_64, integer_64, u64, int64);
        NUMERIC_CONVERTER("Uint64 to Uint8", uinteger_64, uinteger_8, u64, u8);
        NUMERIC_CONVERTER("Uint64 to Uint16", uinteger_64, uinteger_16, u64, u16);
        NUMERIC_CONVERTER("Uint64 to Uint32", uinteger_64, uinteger_32, u64, u32);
        NUMERIC_CONVERTER("Uint64 to Float32", uinteger_64, float_32, u64, f32);
        NUMERIC_CONVERTER("Uint64 to Float64", uinteger_64, float_64, u64, f64);

        NUMERIC_CONVERTER("Float32 to Int8", float_32, integer_8, f32, int8);
        NUMERIC_CONVERTER("Float32 to Int16", float_32, integer_16, f32, int16);
        NUMERIC_CONVERTER("Float32 to Int32", float_32, integer_32, f32, int32);
        NUMERIC_CONVERTER("Float32 to Int64", float_32, integer_64, f32, int64);
        NUMERIC_CONVERTER("Float32 to Uint8", float_32, uinteger_8, f32, u8);
        NUMERIC_CONVERTER("Float32 to Uint16", float_32, uinteger_16, f32, u16);
        NUMERIC_CONVERTER("Float32 to Uint32", float_32, uinteger_32, f32, u32);
        NUMERIC_CONVERTER("Float32 to Uint64", float_32, uinteger_64, f32, u64);
        NUMERIC_CONVERTER("Float32 to Float64", float_32, float_64, f32, f64);

        NUMERIC_CONVERTER("Float64 to Int8", float_64, integer_8, f64, int8);
        NUMERIC_CONVERTER("Float64 to Int16", float_64, integer_16, f64, int16);
        NUMERIC_CONVERTER("Float64 to Int32", float_64, integer_32, f64, int32);
        NUMERIC_CONVERTER("Float64 to Int64", float_64, integer_64, f64, int64);
        NUMERIC_CONVERTER("Float64 to Uint8", float_64, uinteger_8, f64, u8);
        NUMERIC_CONVERTER("Float64 to Uint16", float_64, uinteger_16, f64, u16);
        NUMERIC_CONVERTER("Float64 to Uint32", float_64, uinteger_32, f64, u32);
        NUMERIC_CONVERTER("Float64 to Uint64", float_64, uinteger_64, f64, u64);
        NUMERIC_CONVERTER("Float64 to Float32", float_64, float_32, f64, f32);
        #undef NUMERIC_CONVERTER

        #define TO_STRING_CONVERTER(name, in_pin_type, in_type)                             \
            register_node({name,                                                            \
            "",                                                                             \
            "Convert the input value to a string",                                          \
            "Conversion",                                                                   \
            {pin_data_type::in_pin_type},                                                   \
            {pin_data_type::string},                                                        \
            [](const std::vector<function_value>& inputs) -> std::vector<function_value>    \
            {                                                                               \
                const in_type a = std::get<in_type>(inputs[0]);                             \
                return {AT::util::to_string(a)};                                            \
            }});

        TO_STRING_CONVERTER("Int8 to String", integer_8, int8);
        TO_STRING_CONVERTER("Int16 to String", integer_16, int16);
        TO_STRING_CONVERTER("Int32 to String", integer_32, int32);
        TO_STRING_CONVERTER("Int64 to String", integer_64, int64);
        TO_STRING_CONVERTER("Uint8 to String", uinteger_8, u8);
        TO_STRING_CONVERTER("Uint16 to String", uinteger_16, u16);
        TO_STRING_CONVERTER("Uint32 to String", uinteger_32, u32);
        TO_STRING_CONVERTER("Uint64 to String", uinteger_64, u64);
        TO_STRING_CONVERTER("Float32 to String", float_32, f32);
        TO_STRING_CONVERTER("Float64 to String", float_64, f64);
        TO_STRING_CONVERTER("Bool to String", boolean, bool);

        #undef TO_STRING_CONVERTER

        // String to numeric converters (you'll need to implement from_string in your util)
        #define FROM_STRING_CONVERTER(name, out_pin_type, out_type)                             \
            register_node(minimal_node_definition{                                              \
                name, name,                                                                     \
                "Convert string to " #out_type,                                                 \
                "Conversion",                                                                   \
                {pin_data_type::string},                                                        \
                {pin_data_type::out_pin_type},                                                  \
                [](const std::vector<function_value>& inputs) -> std::vector<function_value>    \
                {                                                                               \
                    const std::string a = std::get<std::string>(inputs[0]);                     \
                    return std::vector<function_value>(AT::util::from_string<out_type>(a));     \
                }})

        FROM_STRING_CONVERTER("String to Int8", integer_8, int8);
        FROM_STRING_CONVERTER("String to Int16", integer_16, int16);
        FROM_STRING_CONVERTER("String to Int32", integer_32, int32);
        FROM_STRING_CONVERTER("String to Int64", integer_64, int64);
        FROM_STRING_CONVERTER("String to Uint8", uinteger_8, u8);
        FROM_STRING_CONVERTER("String to Uint16", uinteger_16, u16);
        FROM_STRING_CONVERTER("String to Uint32", uinteger_32, u32);
        FROM_STRING_CONVERTER("String to Uint64", uinteger_64, u64);
        FROM_STRING_CONVERTER("String to Float32", float_32, f32);
        FROM_STRING_CONVERTER("String to Float64", float_64, f64);
        FROM_STRING_CONVERTER("String to Bool", boolean, bool);

        #undef FROM_STRING_CONVERTER

        // Boolean conversions
        register_node(minimal_node_definition{
            "Int to Bool", "Int to Bool",
            "Convert integer to boolean (0 = false, non-zero = true)",
            "Conversion",
            {pin_data_type::integer_32},
            {pin_data_type::boolean},
            [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                const int32 a = std::get<int32>(inputs[0]);
                return {a != 0};
            }});

        register_node(minimal_node_definition{
            "Bool to Int", "Bool to Int",
            "Convert boolean to integer (false = 0, true = 1)",
            "Conversion",
            {pin_data_type::boolean},
            {pin_data_type::integer_32},
            [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                const bool a = std::get<bool>(inputs[0]);
                return {a ? 1 : 0};
            }});

        register_node(minimal_node_definition{
            "Float to Bool", "Float to Bool",
            "Convert float to boolean (0.0 = false, non-zero = true)",
            "Conversion",
            {pin_data_type::float_32},
            {pin_data_type::boolean},
            [](const std::vector<function_value>& inputs) -> std::vector<function_value> {
                const f32 a = std::get<f32>(inputs[0]);
                return {a != 0.0f};
            }});

            
        #define NODE(input, output, type, operation, name, category)                        \
            register_node({name,                                                            \
            #operation,                                                                     \
            "Compares two numbers (A " + std::string(#operation) + "B)",                    \
            category,                                                                       \
            {pin_data_type::input, pin_data_type::input},                                   \
            {pin_data_type::output},                                                        \
            [](const std::vector<function_value> &inputs) -> std::vector<function_value>    \
            {                                                                               \
                const type a = std::get<type>(inputs[0]);                                   \
                const type b = std::get<type>(inputs[1]);                                   \
                return {a operation b};                                                     \
            }})

        NODE(float_64, boolean, double, ==, "== double", "Logic|Double");
        NODE(float_64, boolean, double, !=, "!= double", "Logic|Double");
        NODE(float_64, boolean, double, > , ">  double", "Logic|Double");
        NODE(float_64, boolean, double, >=, ">= double", "Logic|Double");
        NODE(float_64, boolean, double, < , "<  double", "Logic|Double");
        NODE(float_64, boolean, double, <=, "<= double", "Logic|Double");

        // TODO: need to fix the way its displayed (currently omitting secund same name)
        NODE(integer_32, boolean, int32, ==, "== integer", "Logic|Integer");
        NODE(integer_32, boolean, int32, !=, "!= integer", "Logic|Integer");
        NODE(integer_32, boolean, int32, > , ">  integer", "Logic|Integer");
        NODE(integer_32, boolean, int32, >=, ">= integer", "Logic|Integer");
        NODE(integer_32, boolean, int32, < , "<  integer", "Logic|Integer");
        NODE(integer_32, boolean, int32, <=, "<= integer", "Logic|Integer");

        NODE(boolean, boolean, bool, &&, "&& bool", "Logic|Bool");
        NODE(boolean, boolean, bool, ||, "|| bool", "Logic|Bool");

        NODE(integer_32, integer_32, int32, +, "+ int", "Math|Integer");
        NODE(integer_32, integer_32, int32, -, "- int", "Math|Integer");
        NODE(integer_32, integer_32, int32, *, "* int", "Math|Integer");
        NODE(integer_32, integer_32, int32, /, "/ int", "Math|Integer");
        
        NODE(float_64, float_64, double, +, "+ double", "Math|Double");
        NODE(float_64, float_64, double, -, "- double", "Math|Double");
        NODE(float_64, float_64, double, *, "* double", "Math|Double");
        NODE(float_64, float_64, double, /, "/ double", "Math|Double");
        
        NODE(integer_32, integer_32, int32, +, "+ int", "Math|Integer");
        NODE(integer_32, integer_32, int32, -, "- int", "Math|Integer");
        NODE(integer_32, integer_32, int32, *, "* int", "Math|Integer");

        NODE(integer_32, integer_32, int32, /, "/ int", "Math|Integer");

        NODE(string, string, std::string, +, "Concatenate string", "String");
        NODE(string, boolean, std::string, ==, "Equal", "String");

        #undef NODE

        // register_node({"! <bool>",
        //     "!",
        //     "invert a boolean",
        //     "Logic|Bool",
        //     {pin_data_type::boolean},
        //     {pin_data_type::boolean},
        //     [](const std::vector<function_value> &inputs) -> std::vector<function_value>
        //     {
        //         bool a = std::get<bool>(inputs[0]);
        //         return !(a);
        //     }});

    }

}
