#pragma once

#include "util/util.h"
#include "utils.h"              // node spezific utils

namespace AT
{

    using operation_function = std::function<std::vector<function_value>(const std::vector<function_value>&)>;


    // A minimal node implementation for visual programming with custom drawing
    // minimalNode_group
    // This class provides a simplified node implementation that uses custom drawing
    // for a cleaner visual appearance without traditional headers. It supports
    // various data types and can be configured with custom operations.
    class minimal_node : public ImFlow::BaseNode
    {
    public:
    
        // Constructs a minimalNode with specified parameters
        // name The display name of the node
        // input_types Vector of data types for input pins
        // output_types Vector of data types for output pins
        // operation Optional function that processes inputs to generate outputs
        minimal_node(const std::string &name,
            const std::vector<pin_data_type> &input_types,
            const std::vector<pin_data_type> &output_types,
            const operation_function& operation = nullptr);

        // Empty draw implementation - everything handled by custom drawing
        void draw() override
        {}

        // Indicates that this node uses custom drawing
        bool usesCustomDrawing() const override
        { return true; }

        // Custom drawing implementation for the node
        // draw_list The ImDrawList to draw on
        // offset The offset for drawing coordinates
        void customDraw(ImDrawList *draw_list, const ImVec2 &offset) override;

        // Indicates that this node uses custom hover detection
        bool usesCustomHover() const override
        { return true; }

        // Custom hover detection implementation
        // true if the node is being hovered, false otherwise
        bool customIsHovered() const override;

        // Gets the fixed size of the minimal node
        // Constant reference to the node size
        const ImVec2& getSize() const override;

        // Gets the visual size of the node (same as physical size)
        // The visual size of the node
        ImVec2 getVisualSize() const override;

        // Macro to set node type name for serialization
        SET_NODE_TYPE_NAME(minimal_node)

        // Serializes the node to YAML
        // yaml Reference to the YAML serializer
        void serialize(AT::serializer::yaml &yaml) override;

    private:

        // Internal structure representing a pin in the minimal node
        struct minimal_pin
        {
            pin_data_type                   type;       // Data type of the pin
            std::string                     name;       // Name of the pin
            std::shared_ptr<ImFlow::Pin>    pin_ptr;    // Pointer to the actual pin object
        };

        // Computes the output value for a specific output pin
        // T The type of the output value
        // pin_name The name of the output pin
        // The computed output value of type T
        // This template function collects all input values, executes the operation,
        // and returns the result for the specified output pin.
        template <typename T>
        T compute_output(const std::string &pin_name) {
            if (!m_operation)
                return T{};

            // Collect input values
            std::vector<function_value> inputs;
            for (const auto &pin : m_input_pins) {
                std::visit([&](auto &&default_value) {

                    using U = std::decay_t<decltype(default_value)>;
                    inputs.push_back(this->template getInVal<U>(pin.name));
                }, get_default_value_for_type(pin.type));
            }

            // Call the operation
            auto results = m_operation(inputs);

            // Find the corresponding output
            for (size_t i = 0; i < m_output_pins.size(); ++i)
                if (m_output_pins[i].name == pin_name && i < results.size())
                    return std::get<T>(results[i]);

            return T{};
        }

        // Sets up input and output pins based on provided types
        // input_types Vector of input pin data types
        // output_types Vector of output pin data types
        void setup_pins(const std::vector<pin_data_type> &input_types, const std::vector<pin_data_type> &output_types);

        // Creates a typed pin with appropriate behavior
        // name Name of the pin
        // type Data type of the pin
        // is_input Whether this is an input pin (true) or output pin (false)
        // Shared pointer to the created pin
        std::shared_ptr<ImFlow::Pin> create_typed_pin(const std::string &name, pin_data_type type, bool is_input);

        // Draws a pin socket at the specified position
        // draw_list The ImDrawList to draw on
        // center Center position of the socket
        // type Data type of the pin (determines color)
        // is_input Whether this is an input socket
        void draw_pin_socket(ImDrawList *draw_list, const ImVec2 &center, minimal_pin& pin, bool is_input);

        // Creates the style for minimal nodes
        // Shared pointer to the node style
        std::shared_ptr<ImFlow::NodeStyle> create_minimal_node_style();

        // Checks if a pin socket is being hovered
        // center Center position of the socket in screen coordinates
        // radius Current radius of the socket
        // True if the mouse is hovering the socket
        bool is_pin_hovered(const ImVec2 &center, float radius) const;

        // Gets the hover radius for a pin socket
        // center Center position of the socket
        // baseRadius The base radius when not hovered
        // The radius to use (larger when hovered)
        float get_hover_radius(const ImVec2 &center, float base_radius) const;

        // Gets the currently hovered pin (if any)
        // Pointer to the hovered pin, or nullptr if none
        ImFlow::Pin *get_hovered_pin() const;
        
        // Checks if mouse is over any pin socket
        // true if mouse is over any pin, false otherwise
        bool is_mouse_over_any_pin() const;

        // Checks if mouse is over the node body (excluding pins)
        // true if mouse is over node body, false otherwise
        bool is_mouse_over_node_body() const;

        ImVec2                      m_node_size{};
        mutable int                 m_hovered_pin_index = -1;
        mutable bool                m_hovered_pin_is_input = false;
        std::string                 m_name{};                           // Name of the node
        std::vector<minimal_pin>    m_input_pins{};                     // Vector of input pins
        std::vector<minimal_pin>    m_output_pins{};                    // Vector of output pins
        operation_function          m_operation{};                      // Operation function
        
    };

}
