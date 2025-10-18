#pragma once


namespace AT {

        
    // Define execution data type
    struct Execution {};

    // Custom style creator functions
    static std::shared_ptr<ImFlow::NodeStyle> create_custom_node_style(ImU32 header_color, const char* title = "Node") {
        auto style = std::make_shared<ImFlow::NodeStyle>(
            header_color, 
            ImColor(250, 250, 250, 255), 
            3.0f  // Reduced corner radius
        );
        style->padding = ImVec4(10.f, 0.f, 10.f, 0.f);
        style->bg = ImColor(60, 60, 60, 255);
        style->border_thickness = 1.5f;
        style->border_selected_thickness = 2.5f;
        return style;
    }

}
