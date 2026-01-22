
#pragma once

#include <ImNodeFlow.h>
#include "nodes/utils.h"

namespace AT {

    // Helper function to create the simplified node style
    static std::shared_ptr<ImFlow::NodeStyle> create_simplified_node_style() {
        auto style = std::make_shared<ImFlow::NodeStyle>(
            IM_COL32(80, 80, 80, 255),    // header_bg (same as body for unified look)
            ImColor(220, 220, 220, 255),  // header_title_color (light gray)
            4.0f                          // radius
        );
        style->bg = IM_COL32(60, 60, 60, 255);           // dark gray body
        style->border_color = IM_COL32(120, 120, 120, 255); // brighter gray border
        style->border_selected_color = ImVec4(150, 150, 150, 255);
        style->padding = ImVec4(15.f, 8.f, 15.f, 8.f);   // Adjusted padding for centered title
        return style;
    }

    // PC Node - cannot be deleted, has execution inputs but no outputs
    class PCNode : public ImFlow::BaseNode {
    public:
        PCNode() {
            setTitle("PC");
            setStyle(create_simplified_node_style());
            
            // Add multiple execution inputs for connecting multiple devices
            addIN<ImFlow::Execution>("Device 1", ImFlow::Execution{}, ImFlow::ConnectionFilter::ExecutionOnly(), ImFlow::PinStyle::execution());
            addIN<ImFlow::Execution>("Device 2", ImFlow::Execution{}, ImFlow::ConnectionFilter::ExecutionOnly(), ImFlow::PinStyle::execution());
            addIN<ImFlow::Execution>("Device 3", ImFlow::Execution{}, ImFlow::ConnectionFilter::ExecutionOnly(), ImFlow::PinStyle::execution());
            
            // No outputs - PC only receives execution
        }
        
        // Override destroy to prevent deletion - FIX: Add override keyword
        void destroy() {
            // Do nothing - PC node cannot be deleted
        }
        
        SET_NODE_TYPE_NAME(PCNode)
        
        void serialize(AT::serializer::yaml& yaml) override {
            BaseNode::serialize(yaml);
            // PC node specific serialization if needed
        }
    };

    // Device Node - has execution output
    class DeviceNode : public ImFlow::BaseNode {
    public:
        DeviceNode() {
            setTitle("Device");
            setStyle(create_simplified_node_style());
            
            // Device has execution output
            addOUT<ImFlow::Execution>("Execute", ImFlow::PinStyle::execution())->behaviour([]() { return ImFlow::Execution{}; });
        }
        
        SET_NODE_TYPE_NAME(DeviceNode)
        
        void serialize(AT::serializer::yaml& yaml) override {
            BaseNode::serialize(yaml);
            // Device node specific serialization if needed
        }
    };

    // Node definitions for the simplified editor
    struct SimplifiedNodeDefinition {
        const char* name;
        const char* category;
        std::function<void(ImFlow::ImNodeFlow& editor, const ImVec2& pos)> creator;
        const char* description;
    };

    static std::vector<SimplifiedNodeDefinition> simplified_node_list = {
        {"Device", "Devices",
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<DeviceNode>(pos); },
            "Device with execution output"},
    };
}
