#pragma once

#include <ImNodeFlow.h>
#include "util/ui/panel_collection.h"
#include "util/util.h"

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


    // Execution-only node (like a Begin node)
    class BeginNode : public ImFlow::BaseNode {
    public:

        BeginNode() {

            setTitle("Begin");
            setStyle(create_custom_node_style(IM_COL32(90, 191, 93, 255)));
            addOUT<Execution>("Execute", ImFlow::PinStyle::execution())->behaviour([]() { return Execution{}; });   // Only has execution output
        }
        
        SET_NODE_TYPE_NAME(BeginNode)
        
        void serialize(AT::serializer::yaml& yaml) override {
            BaseNode::serialize(yaml); // Call base serialization
            
            // BeginNode specific serialization
            std::string custom_data = "begin_node_data";
            yaml.entry("custom_data", custom_data);
            
            if (yaml.get_option() == AT::serializer::option::load_from_file) {
                // Load BeginNode specific data
                // custom_data is available here if needed

                // setPos(position);
                // setTitle(title);
            }
        }
    };
    
    // Base math operation node
    class MathNode : public ImFlow::BaseNode {
    public:

        MathNode(const std::string& title) {
            setTitle(title);
            // Use custom style with less rounded corners
            setStyle(create_custom_node_style(IM_COL32(71, 142, 173, 255)));
            
            // Execution pins
            addIN<Execution>("Exec In", Execution{}, ImFlow::ConnectionFilter::SameType(), ImFlow::PinStyle::execution());
            addOUT<Execution>("Exec Out", ImFlow::PinStyle::execution())->behaviour([]() { return Execution{}; });
            
            // Data input pins
            addIN<float>("A", 0.0f, ImFlow::ConnectionFilter::SameType());
            addIN<float>("B", 0.0f, ImFlow::ConnectionFilter::SameType());
            
            // Data output pin - store as typed pointer
            m_resultPin = addOUT<float>("Result");
        }
        
        void draw() override {
            ImGui::PushItemWidth(60);
            ImGui::InputFloat("A", &m_valueA);
            ImGui::InputFloat("B", &m_valueB);
            ImGui::PopItemWidth();
        }
        
        SET_NODE_TYPE_NAME(MathNode)
        
        void serialize(AT::serializer::yaml& yaml) override {
            BaseNode::serialize(yaml);
            
            yaml.entry("value_a", m_valueA)
                .entry("value_b", m_valueB)
                .entry("result", m_result);
                
            if (yaml.get_option() == AT::serializer::option::load_from_file) {
                // Values are automatically loaded by the entry calls above
            }
        }
        
        // Helper method to get the result pin
        std::shared_ptr<ImFlow::OutPin<float>> getResultPin() { return m_resultPin; }

    protected:
        float m_valueA = 0.0f;
        float m_valueB = 0.0f;
        float m_result = 0.0f;
        
        // Store typed pointers to pins for easy access
        std::shared_ptr<ImFlow::OutPin<float>> m_resultPin;
        
    };
    

    class MultiOperationNode : public ImFlow::BaseNode {
    public:
        enum class Operation {
            ADD, SUBTRACT, MULTIPLY, DIVIDE, POWER, SQUARE_ROOT, 
            ABSOLUTE, ROUND, CEIL, FLOOR, MIN, MAX, MODULO, 
            EXPONENTIAL, LOG10, NATURAL_LOG
        };

        MultiOperationNode() {
            setTitle("Multi Operation");
            setStyle(create_custom_node_style(IM_COL32(173, 126, 71, 255)));
            
            // Execution pins
            addIN<Execution>("Exec In", Execution{}, ImFlow::ConnectionFilter::SameType(), ImFlow::PinStyle::execution());
            addOUT<Execution>("Exec Out", ImFlow::PinStyle::execution())->behaviour([]() { return Execution{}; });
            
            // Data input pins
            addIN<float>("A", 0.0f, ImFlow::ConnectionFilter::SameType());
            addIN<float>("B", 1.0f, ImFlow::ConnectionFilter::SameType()); // Default to 1 for division safety
            
            // Data output pin
            m_resultPin = addOUT<float>("Result");
            
            // Initialize with default operation
            updateBehaviour();
        }
        
        void draw() override {
            ImGui::PushItemWidth(120);
            
            // Operation selector
            const char* operations[] = {
                "Add (A+B)", "Subtract (A-B)", "Multiply (A×B)", "Divide (A÷B)", 
                "Power (A^B)", "Square Root (√A)", "Absolute (|A|)", "Round A", 
                "Ceil A", "Floor A", "Minimum (min(A,B))", "Maximum (max(A,B))", 
                "Modulo (A%B)", "Exponential (e^A)", "Log10 A", "Natural Log (ln A)"
            };
            
            if (ImGui::Combo("Operation", reinterpret_cast<int*>(&m_operation), operations, IM_ARRAYSIZE(operations))) {
                updateBehaviour();
            }
            
            ImGui::Spacing();
            
            // Input fields - show both for binary operations, only A for unary
            bool showB = isBinaryOperation(m_operation);
            
            ImGui::InputFloat("A", &m_valueA);
            if (showB) {
                ImGui::InputFloat("B", &m_valueB);
                
                // Safety warning for division by zero
                if (m_operation == Operation::DIVIDE && getInVal<float>("B") == 0.0f) {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Warning: Division by zero!");
                }
            }
            
            ImGui::PopItemWidth();
            
            // Show current operation as text
            ImGui::Spacing();
            ImGui::Text("Current: %s", operations[static_cast<int>(m_operation)]);
            
            // Show result preview
            float result = calculateResult(getInVal<float>("A"), getInVal<float>("B"));
            ImGui::Text("Result: %.4f", result);
        }

        SET_NODE_TYPE_NAME(MultiOperationNode)
        
        void serialize(AT::serializer::yaml& yaml) override {
            BaseNode::serialize(yaml);
            
            yaml.entry("operation", reinterpret_cast<int&>(m_operation))
                .entry("value_a", m_valueA)
                .entry("value_b", m_valueB);
                
            if (yaml.get_option() == AT::serializer::option::load_from_file) {
                updateBehaviour(); // Update the behaviour after loading
            }
        }

    private:
        void updateBehaviour() {
            m_resultPin->behaviour([this]() { 
                float a = getInVal<float>("A");
                float b = getInVal<float>("B");
                return calculateResult(a, b);
            });
        }
        
        float calculateResult(float a, float b) {
            switch (m_operation) {
                case Operation::ADD: return a + b;
                case Operation::SUBTRACT: return a - b;
                case Operation::MULTIPLY: return a * b;
                case Operation::DIVIDE: return b != 0 ? a / b : 0.0f;
                case Operation::POWER: return powf(a, b);
                case Operation::SQUARE_ROOT: return a >= 0 ? sqrtf(a) : 0.0f;
                case Operation::ABSOLUTE: return fabsf(a);
                case Operation::ROUND: return roundf(a);
                case Operation::CEIL: return ceilf(a);
                case Operation::FLOOR: return floorf(a);
                case Operation::MIN: return fminf(a, b);
                case Operation::MAX: return fmaxf(a, b);
                case Operation::MODULO: return b != 0 ? fmodf(a, b) : 0.0f;
                case Operation::EXPONENTIAL: return expf(a);
                case Operation::LOG10: return a > 0 ? log10f(a) : 0.0f;
                case Operation::NATURAL_LOG: return a > 0 ? logf(a) : 0.0f;
                default: return 0.0f;
            }
        }
        
        bool isBinaryOperation(Operation op) {
            // List of operations that require both A and B inputs
            return op == Operation::ADD || op == Operation::SUBTRACT || 
                op == Operation::MULTIPLY || op == Operation::DIVIDE ||
                op == Operation::POWER || op == Operation::MIN || 
                op == Operation::MAX || op == Operation::MODULO;
        }

        Operation m_operation = Operation::ADD;
        float m_valueA = 0.0f;
        float m_valueB = 1.0f;
        std::shared_ptr<ImFlow::OutPin<float>> m_resultPin;
    };


    class PlotterNode : public ImFlow::BaseNode {
    public:
        enum class PlotType {
            LINE_PLOT,
            SCATTER_PLOT,
            BAR_CHART,
            HISTOGRAM,
            HEATMAP,
            PIE_CHART,
            AREA_PLOT,
            ERROR_BARS,
            STEM_PLOT,
            DIGITAL_PLOT
        };

        PlotterNode() {
            setTitle("Plotter");
            setStyle(create_custom_node_style(IM_COL32(142, 68, 173, 255)));
            
            // Execution pins
            addIN<Execution>("Exec In", Execution{}, ImFlow::ConnectionFilter::SameType(), ImFlow::PinStyle::execution());
            addOUT<Execution>("Exec Out", ImFlow::PinStyle::execution())->behaviour([]() { return Execution{}; });
            
            // Data input pins
            addIN<std::vector<float>>("X Data", std::vector<float>{}, ImFlow::ConnectionFilter::SameType());
            addIN<std::vector<float>>("Y Data", std::vector<float>{}, ImFlow::ConnectionFilter::SameType());
            addIN<std::vector<float>>("Z Data", std::vector<float>{}, ImFlow::ConnectionFilter::SameType());
            
            // Initialize with sample data
            generateSampleData();
        }
        
        void draw() override {
            ImGui::PushItemWidth(120);
            
            // Plot type selector
            const char* plot_types[] = {
                "Line Plot", "Scatter Plot", "Bar Chart", "Histogram", "Heatmap",
                "Pie Chart", "Area Plot", "Error Bars", "Stem Plot", "Digital Plot"
            };
            ImGui::Combo("Plot Type", reinterpret_cast<int*>(&m_plotType), plot_types, IM_ARRAYSIZE(plot_types));
            
            ImGui::Spacing();
            
            // Preview toggle
            ImGui::Checkbox("Show Preview", &m_showPreview);
            
            // Data info
            auto x_data = getInVal<std::vector<float>>("X Data");
            auto y_data = getInVal<std::vector<float>>("Y Data");
            
            ImGui::Text("X Data: %zu points", x_data.size());
            ImGui::Text("Y Data: %zu points", y_data.size());
            
            if (x_data.size() != y_data.size() && !x_data.empty() && !y_data.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Warning: X/Y data size mismatch!");
            }
            
            ImGui::PopItemWidth();
            
            // Preview section
            if (m_showPreview) {
                drawPreview();
            }
        }
        
        SET_NODE_TYPE_NAME(PlotterNode)
        
        void serialize(AT::serializer::yaml& yaml) override {
            BaseNode::serialize(yaml);
            
            yaml.entry("plot_type", reinterpret_cast<int&>(m_plotType))
                .entry("show_preview", m_showPreview);
        }
        
    private:
        void generateSampleData() {
            // Generate sample sine wave data
            m_sampleXData.clear();
            m_sampleYData.clear();
            
            for (int i = 0; i < 100; ++i) {
                float x = i * 0.1f;
                m_sampleXData.push_back(x);
                m_sampleYData.push_back(sinf(x) + 0.1f * (rand() / (float)RAND_MAX - 0.5f));
            }
            
            // Generate sample 2D data for heatmap
            m_sampleZData.clear();
            for (int i = 0; i < 10; ++i) {
                for (int j = 0; j < 10; ++j) {
                    m_sampleZData.push_back(sinf(i * 0.5f) * cosf(j * 0.5f));
                }
            }
        }
        
        void drawPreview() {
            ImGui::Spacing();
            ImGui::Text("Preview:");
            
            // Get data (use sample data if inputs are empty)
            auto x_data = getInVal<std::vector<float>>("X Data");
            auto y_data = getInVal<std::vector<float>>("Y Data");
            auto z_data = getInVal<std::vector<float>>("Z Data");
            
            if (x_data.empty() || y_data.empty()) {
                x_data = m_sampleXData;
                y_data = m_sampleYData;
            }
            
            if (z_data.empty()) {
                z_data = m_sampleZData;
            }
            
            // Create preview plot
            ImVec2 preview_size = ImVec2(300, 200);
            
            if (ImPlot::BeginPlot("##Preview", preview_size, ImPlotFlags_CanvasOnly)) {
                ImPlot::SetupAxes("X", "Y");
                ImPlot::SetupAxesLimits(0, 10, -1.5, 1.5);
                
                switch (m_plotType) {
                    case PlotType::LINE_PLOT:
                        if (!x_data.empty() && !y_data.empty()) {
                            ImPlot::PlotLine("Data", x_data.data(), y_data.data(), 
                                        std::min(x_data.size(), y_data.size()));
                        }
                        break;
                        
                    case PlotType::SCATTER_PLOT:
                        if (!x_data.empty() && !y_data.empty()) {
                            ImPlot::PlotScatter("Data", x_data.data(), y_data.data(), 
                                            std::min(x_data.size(), y_data.size()));
                        }
                        break;
                        
                    case PlotType::BAR_CHART:
                        if (!y_data.empty()) {
                            ImPlot::PlotBars("Data", y_data.data(), y_data.size());
                        }
                        break;
                        
                    case PlotType::HISTOGRAM:
                        if (!y_data.empty()) {
                            ImPlot::PlotHistogram("Data", y_data.data(), y_data.size());
                        }
                        break;
                        
                    case PlotType::HEATMAP:
                        if (!z_data.empty()) {
                            ImPlot::PlotHeatmap("Data", z_data.data(), 10, 10);
                        }
                        break;
                        
                    case PlotType::PIE_CHART:
                        if (!y_data.empty() && y_data.size() >= 3) {
                            const char* labels[] = {"Slice 1", "Slice 2", "Slice 3"};
                            ImPlot::PlotPieChart(labels, y_data.data(), 3, 0.5f, 0.5f, 0.4f);
                        }
                        break;
                        
                    case PlotType::AREA_PLOT:
                        if (!x_data.empty() && !y_data.empty()) {
                            ImPlot::PlotShaded("Data", x_data.data(), y_data.data(), 
                                            std::min(x_data.size(), y_data.size()));
                        }
                        break;
                        
                    case PlotType::ERROR_BARS:
                        if (!x_data.empty() && !y_data.empty()) {
                            std::vector<float> err(y_data.size(), 0.1f);
                            ImPlot::PlotErrorBars("Data", x_data.data(), y_data.data(), 
                                                err.data(), std::min(x_data.size(), y_data.size()));
                        }
                        break;
                        
                    case PlotType::STEM_PLOT:
                        if (!x_data.empty() && !y_data.empty()) {
                            ImPlot::PlotStems("Data", x_data.data(), y_data.data(), 
                                        std::min(x_data.size(), y_data.size()));
                        }
                        break;
                        
                    case PlotType::DIGITAL_PLOT:
                        if (!x_data.empty() && !y_data.empty()) {
                            ImPlot::PlotDigital("Data", x_data.data(), y_data.data(), 
                                            std::min(x_data.size(), y_data.size()));
                        }
                        break;
                }
                
                ImPlot::EndPlot();
            }
            
            // Plot description
            const char* descriptions[] = {
                "Continuous line connecting data points",
                "Individual points without connecting lines", 
                "Rectangular bars with heights proportional to values",
                "Distribution of numerical data",
                "2D data visualization using colors",
                "Circular statistical graphic divided into slices",
                "Area under the line curve is filled",
                "Data points with error margins",
                "Lines from baseline to data points",
                "Digital signal representation"
            };
            
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", 
                            descriptions[static_cast<int>(m_plotType)]);
        }

        PlotType m_plotType = PlotType::LINE_PLOT;
        bool m_showPreview = true;
        
        // Sample data for preview
        std::vector<float> m_sampleXData;
        std::vector<float> m_sampleYData;
        std::vector<float> m_sampleZData;
    };
    

    class comment_node : public ImFlow::BaseNode {
    public:

        comment_node() {

            m_comment_text = "Double click to edit comment...";
            m_isEditing = false;
            m_addingNodesMode = false;
            m_customSize = ImVec2(200, 100);
            m_padding = 20.0f;
            m_posTarget = getPos();
            
            setTitle("Comment");
            setStyle(create_custom_node_style(IM_COL32(200, 160, 60, 255), "Comment"));
            setColor(ImVec4(0.f, 0.f, 0.f, 1.0f));
        }
        

        void draw() override { /* No content in the main body */ }
        

        bool isCommentNode() const override { return true; }


        bool usesCustomDrawing() const override { return true; }
        
        
        bool usesCustomHover() const override { return true; }
        
                        
        bool customIsHovered() const override {
            auto handler = getHandler();
            if (!handler) return false;
            
            ImVec2 pos = getPos();
            ImVec2 size = getSize();
            
            // Convert to screen coordinates for accurate hover detection
            ImVec2 screenMin = handler->grid2screen(pos);
            
            // Only the header area (top portion) should be hoverable for dragging
            float headerHeight = 25.0f;
            ImVec2 headerMax = ImVec2(screenMin.x + size.x, screenMin.y + headerHeight);
            
            return ImGui::IsMouseHoveringRect(screenMin, headerMax);
        }
                

        void handleDragging() {
            auto handler = getHandler();
            if (!handler) return;

            bool mouseClickState = handler->getSingleUseClick();
            
            // Only handle dragging if we're actually hovered (over header/border)
            if (isHovered() && mouseClickState && !handler->isBoxSelecting()) {
                handler->consumeSingleUseClick();
                m_dragged = true;
                handler->draggingNode(true);
                
                // Initialize m_posTarget to current position when starting drag
                m_posTarget = getPos();
            }
            
            if (m_dragged || (isSelected() && handler->isNodeDragged())) {
                float step = handler->getStyle().grid_size / handler->getStyle().grid_subdivisions;
                
                // Convert mouse delta from screen space to grid space
                ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
                if (handler->getGrid().scale() > 0) {
                    mouseDelta.x /= handler->getGrid().scale();
                    mouseDelta.y /= handler->getGrid().scale();
                }
                
                ImVec2 oldPos = getPos(); // Store position before update
                m_posTarget += mouseDelta;
                
                // "Slam" The position to grid
                ImVec2 newPos = ImVec2(round(m_posTarget.x / step) * step, round(m_posTarget.y / step) * step);
                setPos(newPos);
                
                // Update contained nodes by the same delta (including nested comments)
                ImVec2 delta = newPos - oldPos;
                if (delta.x != 0 || delta.y != 0) {
                    moveContainedNodes(delta, step);
                    
                    // Update our bounds after moving (in case nested comments changed position)
                    updateCommentBounds();
                }

                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    m_dragged = false;
                    handler->draggingNode(false);
                    m_posTarget = getPos(); // Reset to actual position after dragging
                    
                    // Final bounds update after dragging stops
                    updateCommentBounds();
                }
            }
        }


        void updateCommentBounds() {
            if (m_containedNodes.empty()) {
                // If no contained nodes, use default size
                m_customSize = ImVec2(200, 100);
                return;
            }
            
            // Calculate bounds that encompass all contained nodes (including nested comments)
            float minX = FLT_MAX, minY = FLT_MAX;
            float maxX = -FLT_MAX, maxY = -FLT_MAX;
            
            auto handler = getHandler();
            if (!handler) return;
            
            // Track nodes we've already processed to avoid infinite recursion
            std::set<ImFlow::NodeUID> processedNodes;
            calculateRecursiveBounds(processedNodes, minX, minY, maxX, maxY);
            
            // Add OUR padding around the contained nodes (not including inner comment padding)
            // The inner comments already have their own padding included in their bounds
            ImVec2 newPos = ImVec2(minX - 10.f - m_padding, minY - 30.f - m_padding);
            ImVec2 newSize = ImVec2((maxX - minX) + (2 * m_padding), (maxY - minY) + 30.f + (2 * m_padding));
            
            // Update our position and size
            setPos(newPos);
            m_customSize = newSize;
        }

        // Override getSize to return our custom size
        const ImVec2& getSize() const override { return m_customSize; }
        
        // Override getVisualSize for bounds calculation
        ImVec2 getVisualSize() const override { return m_customSize; }
        
        //
        void add_contained_node(ImFlow::NodeUID nodeId) { 
            // Prevent circular references
            if (wouldCreateCircularReference(nodeId)) {
                // Log warning or show message
                LOG(Warn, "Cannot add node to comment: would create circular reference")
                return;
            }
            m_containedNodes.insert(nodeId); 
        }
        
        //
        void remove_contained_node(ImFlow::NodeUID nodeId) { m_containedNodes.erase(nodeId); }
        
        //
        void clear_contained_nodes() { m_containedNodes.clear(); }
        
        //
        const std::set<ImFlow::NodeUID>& get_contained_nodes() const { return m_containedNodes; }
        
        //

        void customDraw(ImDrawList* draw_list, const ImVec2& offset) override {
            
            auto handler = getHandler();
            if (!handler) return;
            
            updateCommentBounds();
            ImVec2 screenPos = handler->grid2screen(getPos());
            ImVec2 size = getSize();
                    
            // Handle dragging first
            handleDragging();
            
            // Draw main comment box (semi-transparent background with border)
            ImU32 bgColor = m_bgColor;
            ImU32 borderColor = m_borderColor;
            ImU32 headerBgColor = m_headerColor;
            
            // If in adding mode, highlight the border
            if (m_addingNodesMode) {
                borderColor = IM_COL32(255, 255, 0, 255); // Yellow highlight
            }
            
            // Main box
            draw_list->AddRectFilled(screenPos, screenPos + size, bgColor, 2.0f);
            draw_list->AddRect(screenPos, screenPos + size, borderColor, 2.0f, 0, m_addingNodesMode ? 3.0f : 2.0f);
            
            // Header box (for comment text)
            float headerHeight = 25.0f;
            ImVec2 headerSize = ImVec2(size.x, headerHeight);
            draw_list->AddRectFilled(screenPos, screenPos + headerSize, headerBgColor, 3.0f, ImDrawFlags_RoundCornersTop);
            
            // Draw comment text
            ImGui::SetCursorScreenPos(screenPos + ImVec2(5.0f, 5.0f));
            ImGui::PushItemWidth(size.x - 70.0f); // Leave space for buttons
            
            if (m_isEditing) {
                char buffer[256];
                strncpy(buffer, m_comment_text.c_str(), sizeof(buffer));
                if (ImGui::InputText("##CommentText", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                    m_comment_text = buffer;
                    m_isEditing = false;
                }
                // Stop editing if click outside
                if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered()) {
                    m_isEditing = false;
                }
            } else {
                ImGui::TextUnformatted(m_comment_text.c_str());
                // Double click to edit
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                    m_isEditing = true;
                }
            }
            
            ImGui::PopItemWidth();
            
            // Draw ADD button (left of settings button)
            ImVec2 addButtonPos = screenPos + ImVec2(size.x - 50.0f, 5.0f);
            ImGui::SetCursorScreenPos(addButtonPos);
            
            // Style the add button - highlight if in adding mode
            ImU32 addButtonColor = m_addingNodesMode ? IM_COL32(100, 255, 100, 255) : IM_COL32(0, 0, 0, 0);
            ImU32 addButtonHoveredColor = m_addingNodesMode ? IM_COL32(150, 255, 150, 255) : IM_COL32(255, 255, 255, 30);
            ImU32 addButtonActiveColor = m_addingNodesMode ? IM_COL32(200, 255, 200, 255) : IM_COL32(255, 255, 255, 50);
            
            ImGui::PushStyleColor(ImGuiCol_Button, addButtonColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, addButtonHoveredColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, addButtonActiveColor);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 200));
            
            if (ImGui::Button("+", ImVec2(20.0f, 15.0f))) {
                m_addingNodesMode = !m_addingNodesMode; // Toggle mode
                m_selectedNodesDuringAdd.clear(); // Clear previous selection
            }
            
            // Add button tooltip
            if (ImGui::IsItemHovered()) {
                if (m_addingNodesMode) {
                    ImGui::SetTooltip("Adding nodes mode active\n- Click nodes to add to comment\n- Hold SHIFT for multiple selection\n- Right-click or press + again to cancel");
                } else {
                    ImGui::SetTooltip("Add nodes to this comment\nClick to enter selection mode");
                }
            }
            
            // Draw settings button in top-right corner
            ImVec2 settingsButtonPos = screenPos + ImVec2(size.x - 25.0f, 5.0f);
            ImGui::SetCursorScreenPos(settingsButtonPos);
            
            if (ImGui::Button("...", ImVec2(20.0f, 15.0f))) {
                m_showSettings = !m_showSettings;
            }
            
            ImGui::PopStyleColor(4);
            
            // Settings tooltip
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Comment Settings");
            }
            
            // Handle node selection when in adding mode
            if (m_addingNodesMode) {
                handleNodeSelectionMode();
            }
            
            // Draw settings window if open
            if (m_showSettings) {
                drawSettingsWindow(screenPos, size);
            }
        }

        //
        void drawSettingsWindow(const ImVec2& screenPos, const ImVec2& size) {

            // Position the settings window near the comment
            ImVec2 settingsPos = screenPos + ImVec2(size.x + 5.0f, 0.0f);
            ImGui::SetNextWindowPos(settingsPos, ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(200, 0), ImGuiCond_Appearing);
            
            ImGui::Begin("Comment Settings", &m_showSettings, 
                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                        ImGuiWindowFlags_AlwaysAutoResize);
            
            // Color picker for the main color
            ImGui::Text("Comment Color");
            ImVec4 color = ImColor(m_headerColor);
            if (ImGui::ColorEdit4("##CommentColor", (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar)) {
                setColor(color);
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Padding slider
            ImGui::Text("Padding");
            ImGui::SetNextItemWidth(150.0f);
            if (ImGui::SliderFloat("##Padding", &m_padding, 5.0f, 50.0f, "%.0f px")) {
                // Padding changed, update bounds
                updateCommentBounds();
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Quick color presets
            ImGui::Text("Presets:");
            ImGui::BeginGroup();
            if (ImGui::ColorButton("White", ImVec4(0.78f, 0.63f, 0.24f, 1.0f))) {
                setColor(ImVec4(1.f, 1.f, 1.f, 1.0f));
            }
            ImGui::SameLine();
            if (ImGui::ColorButton("Black", ImVec4(0.78f, 0.63f, 0.24f, 1.0f))) {
                setColor(ImVec4(0.f, 0.f, 0.f, 1.0f));
            }
            ImGui::SameLine();
            if (ImGui::ColorButton("Amber", ImVec4(0.78f, 0.63f, 0.24f, 1.0f))) {
                setColor(ImVec4(0.78f, 0.63f, 0.24f, 1.0f));
            }
            ImGui::SameLine();
            if (ImGui::ColorButton("Blue", ImVec4(0.2f, 0.4f, 0.8f, 1.0f))) {
                setColor(ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
            }
            ImGui::SameLine();
            if (ImGui::ColorButton("Green", ImVec4(0.2f, 0.7f, 0.3f, 1.0f))) {
                setColor(ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
            }
            ImGui::EndGroup();
            
            ImGui::End();
        }
        
        //
        void setColor(const ImVec4& color) {
            // Convert ImVec4 to ImU32 and set all color components
            m_headerColor = ImColor(color.x, color.y, color.z, color.w * 0.7f); // Header is less transparent
            m_bgColor = ImColor(color.x, color.y, color.z, color.w * 0.2f);     // Background is more transparent
            m_borderColor = ImColor(color.x, color.y, color.z, color.w);        // Border uses full alpha
        }
        
        //
        void setCommentText(const std::string& text) { m_comment_text = text; }
        
        // 
        bool isInAddingMode() const { return m_addingNodesMode; }

        //
        const std::string& getCommentText() const { return m_comment_text; }
            
        SET_NODE_TYPE_NAME(comment_node)
        

        bool wouldCreateCircularReference(ImFlow::NodeUID potentialChildId) {
            // A node cannot contain itself
            if (potentialChildId == getUID()) return true;
            
            auto handler = getHandler();
            if (!handler) return false;
            
            // Check if the potential child is already our ancestor
            std::set<ImFlow::NodeUID> visited;
            return isAncestor(potentialChildId, visited);
        }


        int getNestingDepth() const {
            auto handler = getHandler();
            if (!handler) return 0;
            
            int maxDepth = 0;
            for (auto nodeId : m_containedNodes) {
                auto& nodes = handler->getNodes();
                auto it = nodes.find(nodeId);
                if (it != nodes.end()) {
                    if (auto nestedComment = std::dynamic_pointer_cast<comment_node>(it->second)) {
                        maxDepth = std::max(maxDepth, nestedComment->getNestingDepth() + 1);
                    }
                }
            }
            return maxDepth;
        }


        ImVec2 getVisualBoundsMin() const { return getPos(); }


        ImVec2 getVisualBoundsMax() const { return getPos() + getVisualSize(); }

        //
        void serialize(AT::serializer::yaml& yaml) override {
            BaseNode::serialize(yaml);
            
            yaml.entry("comment_text", m_comment_text)
                .entry("padding", m_padding)
                .entry("custom_size_x", m_customSize.x)
                .entry("custom_size_y", m_customSize.y);
                
            // Serialize contained nodes
            std::vector<ImFlow::NodeUID> contained_nodes(m_containedNodes.begin(), m_containedNodes.end());
            yaml.entry("contained_nodes", contained_nodes);
            
            if (yaml.get_option() == AT::serializer::option::load_from_file) {
                for (const auto node : contained_nodes)
                    m_containedNodes.insert(node);
            }
        }


        void serializePins(AT::serializer::yaml& yaml) override {
            // Comment nodes don't have pins, but we need to update bounds after loading
            if (yaml.get_option() == AT::serializer::option::load_from_file) {
                // Don't update bounds yet - wait for postLoadFixup
            }
        }

    private:

        void calculateRecursiveBounds(std::set<ImFlow::NodeUID>& processed, float& minX, float& minY, float& maxX, float& maxY) {
            auto handler = getHandler();
            if (!handler) return;
            
            for (auto nodeId : m_containedNodes) {
                // Avoid infinite recursion
                if (processed.count(nodeId) > 0) continue;
                processed.insert(nodeId);
                
                auto& nodes = handler->getNodes();
                auto it = nodes.find(nodeId);
                if (it != nodes.end()) {
                    auto node = it->second;
                    ImVec2 nodePos = node->getPos();
                    ImVec2 nodeSize = node->getFullSize();
                    
                    // For comment nodes, we need to include their visual bounds (position + size)
                    // which already includes their padding
                    if (auto nestedComment = std::dynamic_pointer_cast<comment_node>(node)) {
                        // Comment nodes have their padding already calculated in their size
                        // So we just use their current position and visual size
                        ImVec2 commentVisualSize = nestedComment->getVisualSize();
                        
                        minX = std::min(minX, nodePos.x);
                        minY = std::min(minY, nodePos.y);
                        maxX = std::max(maxX, nodePos.x + commentVisualSize.x);
                        maxY = std::max(maxY, nodePos.y + commentVisualSize.y);
                        
                        // Recursively calculate bounds of nested comment's contained nodes
                        nestedComment->calculateRecursiveBounds(processed, minX, minY, maxX, maxY);
                    } else {
                        // Regular nodes - use their position and full size
                        minX = std::min(minX, nodePos.x);
                        minY = std::min(minY, nodePos.y);
                        maxX = std::max(maxX, nodePos.x + nodeSize.x);
                        maxY = std::max(maxY, nodePos.y + nodeSize.y);
                    }
                }
            }
        }


        void handleNodeSelectionMode() {
            auto handler = getHandler();
            if (!handler) return;
            
            // Check for right-click to cancel
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                m_addingNodesMode = false;
                m_selectedNodesDuringAdd.clear();
                return;
            }
            
            // Get shift state at the beginning so it's available throughout the function
            bool shiftHeld = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
            
            // Check for shift key release to confirm selection
            if ((ImGui::IsKeyReleased(ImGuiKey_LeftShift) || ImGui::IsKeyReleased(ImGuiKey_RightShift)) && 
                !m_selectedNodesDuringAdd.empty()) {
                // Add all selected nodes and exit mode
                for (auto nodeId : m_selectedNodesDuringAdd) {
                    add_contained_node(nodeId);
                }
                m_addingNodesMode = false;
                m_selectedNodesDuringAdd.clear();
                updateCommentBounds(); // Update bounds after adding nodes
                return;
            }
            
            // Get all nodes from the editor
            auto& allNodes = handler->getNodes();
            
            // Check for left-click on nodes
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                // Find which node was clicked (if any)
                for (auto& [nodeId, node] : allNodes) {
                    // Skip ourselves
                    if (node.get() == this) {
                        continue;
                    }
                    
                    // Check if this node is hovered
                    if (node->isHovered()) {
                        // Allow selecting comment nodes, but check for circular references
                        if (auto clickedComment = std::dynamic_pointer_cast<comment_node>(node)) {
                            // Check if this would create a circular reference
                            if (wouldCreateCircularReference(nodeId)) {
                                // Show warning or skip
                                LOG(Warn, "Cannot add comment to comment: would create circular reference")
                                continue;
                            }
                        }
                        
                        if (shiftHeld) {
                            // Toggle selection in multi-select mode
                            if (m_selectedNodesDuringAdd.count(nodeId) > 0) {
                                m_selectedNodesDuringAdd.erase(nodeId);
                            } else {
                                m_selectedNodesDuringAdd.insert(nodeId);
                            }
                        } else {
                            // Single selection - add this node and exit mode
                            add_contained_node(nodeId);
                            m_addingNodesMode = false;
                            m_selectedNodesDuringAdd.clear();
                            updateCommentBounds(); // Update bounds after adding node
                        }
                        break; // Only handle one node per click
                    }
                }
                
                // If we're in multi-select mode and clicked on empty space, check if we should apply selection
                if (shiftHeld && handler->on_free_space() && !m_selectedNodesDuringAdd.empty()) {
                    // Add all selected nodes
                    for (auto nodeId : m_selectedNodesDuringAdd) {
                        add_contained_node(nodeId);
                    }
                    m_addingNodesMode = false;
                    m_selectedNodesDuringAdd.clear();
                    updateCommentBounds(); // Update bounds after adding nodes
                }
            }
            
            // Visual feedback for selected nodes during add mode
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            for (auto& [nodeId, node] : allNodes) {
                if (node.get() == this) {
                    continue;
                }
                
                ImVec2 nodeScreenPos, nodeSize;
                
                if (auto comment = std::dynamic_pointer_cast<comment_node>(node)) {
                    // For comment nodes, use their visual bounds
                    nodeScreenPos = handler->grid2screen(comment->getPos()) - ImVec2(15, 5);
                    nodeSize = comment->getVisualSize() + ImVec2(10, 10);
                    
                    // Special visual indicator for comment nodes
                    if (m_selectedNodesDuringAdd.count(nodeId) > 0) {
                        draw_list->AddRect(nodeScreenPos, nodeScreenPos + nodeSize, IM_COL32(100, 255, 100, 255), 0.0f, 0, 3.0f);
                        // Add text indicator that this is a comment
                        ImVec2 textPos = nodeScreenPos + ImVec2(5, -20);
                        draw_list->AddText(textPos, IM_COL32(100, 255, 100, 255), "Comment");
                    } else if (node->isHovered()) {
                        draw_list->AddRect(nodeScreenPos, nodeScreenPos + nodeSize, IM_COL32(255, 255, 100, 255), 0.0f, 0, 2.0f);
                        // Add text indicator that this is a comment
                        ImVec2 textPos = nodeScreenPos + ImVec2(5, -20);
                        draw_list->AddText(textPos, IM_COL32(255, 255, 100, 255), "Comment");
                    }
                } else {
                    // For regular nodes
                    nodeScreenPos = handler->grid2screen(node->getPos()) - ImVec2(15, 5);
                    nodeSize = node->getFullSize() + ImVec2(10, 10);
                    
                    if (m_selectedNodesDuringAdd.count(nodeId) > 0) {
                        draw_list->AddRect(nodeScreenPos, nodeScreenPos + nodeSize, IM_COL32(100, 255, 100, 255), 0.0f, 0, 3.0f);
                    } else if (node->isHovered()) {
                        draw_list->AddRect(nodeScreenPos, nodeScreenPos + nodeSize, IM_COL32(255, 255, 100, 255), 0.0f, 0, 2.0f);
                    }
                }
            }
            
            // Draw instruction text
            ImVec2 screenPos = handler->grid2screen(getPos());
            ImVec2 instructionPos = screenPos + ImVec2(5, getSize().y + 5);
            std::string instruction;
            
            if (!m_selectedNodesDuringAdd.empty()) {
                instruction = shiftHeld ? 
                    "SHIFT: Release SHIFT to confirm selection, click empty space to confirm" : 
                    "Release SHIFT to confirm selection, or click empty space to confirm";
            } else {
                instruction = shiftHeld ? 
                    "SHIFT: Click nodes to multi-select, release SHIFT to confirm" : 
                    "Click nodes to add to comment, hold SHIFT for multiple";
            }
            
            // Add note about comment nesting
            instruction += "\nComments can contain other comments (no circular references)";
            
            draw_list->AddText(instructionPos, IM_COL32(255, 255, 255, 255), instruction.c_str());
        }


        void moveContainedNodes(const ImVec2& delta, float step) {
            auto handler = getHandler();
            if (!handler) return;
            
            // Track nodes we've already moved to avoid moving them multiple times
            std::set<ImFlow::NodeUID> movedNodes;
            moveContainedNodesRecursive(delta, step, movedNodes);
        }


        void moveContainedNodesRecursive(const ImVec2& delta, float step, std::set<ImFlow::NodeUID>& movedNodes) {
            auto handler = getHandler();
            if (!handler) return;
            
            for (auto nodeId : m_containedNodes) {
                // Avoid moving nodes multiple times
                if (movedNodes.count(nodeId) > 0) continue;
                movedNodes.insert(nodeId);
                
                auto& nodes = handler->getNodes();
                auto it = nodes.find(nodeId);
                if (it != nodes.end()) {
                    auto node = it->second;
                    ImVec2 nodeNewPos = node->getPos() + delta;
                    // Snap to grid
                    nodeNewPos = ImVec2(round(nodeNewPos.x / step) * step, round(nodeNewPos.y / step) * step);
                    node->setPos(nodeNewPos);
                    
                    // If this is a nested comment, recursively move its contained nodes
                    if (auto nestedComment = std::dynamic_pointer_cast<comment_node>(node)) {
                        nestedComment->moveContainedNodesRecursive(delta, step, movedNodes);
                    }
                }
            }
        }


        bool isAncestor(ImFlow::NodeUID nodeId, std::set<ImFlow::NodeUID>& visited) {
            if (visited.count(nodeId) > 0) return false;
            visited.insert(nodeId);
            
            // This node is an ancestor if nodeId contains our UID
            auto handler = getHandler();
            if (!handler) return false;
            
            auto& nodes = handler->getNodes();
            auto it = nodes.find(nodeId);
            if (it != nodes.end()) {
                if (auto potentialAncestor = std::dynamic_pointer_cast<comment_node>(it->second)) {
                    if (potentialAncestor->get_contained_nodes().count(getUID()) > 0) {
                        return true;
                    }
                    // Recursively check the potential ancestor's contained nodes
                    for (auto containedId : potentialAncestor->get_contained_nodes()) {
                        if (isAncestor(containedId, visited)) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }


        std::set<ImFlow::NodeUID>       m_containedNodes;
        std::string                     m_comment_text;
        bool                            m_isEditing = false;
        bool                            m_showSettings = false;
        ImVec2                          m_customSize;  // Custom size for comment node
        bool                            m_dragged{};
        ImVec2                          m_posTarget{};
        bool                            m_addingNodesMode = false;  // track if we're in add mode
        std::set<ImFlow::NodeUID>       m_selectedNodesDuringAdd;   // track nodes selected during add mode

        // appearance settings
        ImU32                           m_headerColor;
        ImU32                           m_bgColor;
        ImU32                           m_borderColor;
        f32                             m_padding = 20.0f;
    };

    // Specific math operation nodes (AddNode, MultiplyNode, SubtractNode remain the same...)
    class AddNode : public MathNode {
    public:
        AddNode() : MathNode("Add") {
            getResultPin()->behaviour([this]() { 
                m_result = getInVal<float>("A") + getInVal<float>("B"); 
                return m_result; 
            });
        }
        
        void draw() override {
            MathNode::draw();
        }

        SET_NODE_TYPE_NAME(AddNode)

    };

    
    class MultiplyNode : public MathNode {
    public:
        MultiplyNode() : MathNode("Multiply") {
            getResultPin()->behaviour([this]() { 
                m_result = getInVal<float>("A") * getInVal<float>("B"); 
                return m_result; 
            });
        }
        
        void draw() override {
            MathNode::draw();
        }

        SET_NODE_TYPE_NAME(MultiplyNode)

    };
    

    class SubtractNode : public MathNode {
    public:
        SubtractNode() : MathNode("Subtract") {
            getResultPin()->behaviour([this]() { 
                m_result = getInVal<float>("A") - getInVal<float>("B"); 
                return m_result; 
            });
        }
        
        void draw() override {
            MathNode::draw();
        }

        SET_NODE_TYPE_NAME(SubtractNode)
        
    };


    // Define all available nodes with categories
    struct NodeDefinition {
        const char* name;
        const char* category;
        // std::function<std::shared_ptr<ImFlow::BaseNode>(ImFlow::ImNodeFlow& editor, const ImVec2& pos)> creator;
        std::function<void(ImFlow::ImNodeFlow& editor, const ImVec2& pos)> creator;
        const char* description;
    };

    static std::vector<NodeDefinition> node_list = {
        {"Begin", "Execution",
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<BeginNode>(pos); },
            "Start execution flow"},
        
        {"Add", "Math Operations",
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<AddNode>(pos); },
            "A + B"},
        
        {"Multiply", "Math Operations",
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<MultiplyNode>(pos); },
            "A × B"},
        
        {"Subtract", "Math Operations", 
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<SubtractNode>(pos); }, 
            "A - B"},
        
        {"Multi Operation", "Math Operations", 
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<MultiOperationNode>(pos); },
            "16 math operations in one node"},
        
        {"Plotter", "Visualization", 
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<PlotterNode>(pos); },
            "Create various types of plots and charts"},

        {"Comment", "Organization",
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<comment_node>(pos); },
            "Group nodes with a comment box"},
    };

}
