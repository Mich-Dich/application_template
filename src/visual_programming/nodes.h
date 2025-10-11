#pragma once

#include <ImNodeFlow.h>
#include "util/ui/panel_collection.h"

namespace AT {
    
    // Define execution data type
    struct Execution {};

    // Custom style creator functions
    static std::shared_ptr<ImFlow::NodeStyle> createCustomNodeStyle(ImU32 header_color, const char* title = "Node") {
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

    // Custom background style
    static void setupCustomBackground(ImFlow::ImNodeFlow& editor) {
        auto& style = editor.getStyle();
        style.colors.background = IM_COL32(200, 44, 52, 255);
        style.colors.grid = IM_COL32(65, 70, 80, 60);
        style.colors.subGrid = IM_COL32(55, 60, 70, 30);
        style.grid_size = 100.0f;
        style.grid_subdivisions = 10.0f;
        // editor.update();
    }

    // Execution-only node (like a Begin node)
    class BeginNode : public ImFlow::BaseNode {
    public:
        BeginNode() {

            setTitle("Begin");
            setStyle(createCustomNodeStyle(IM_COL32(90, 191, 93, 255)));
            
            // Only has execution output
            addOUT<Execution>("Execute", ImFlow::PinStyle::execution())->behaviour([]() { return Execution{}; });
        }
        
    };
    
    // Base math operation node
    class MathNode : public ImFlow::BaseNode {
    public:

        MathNode(const std::string& title) {
            setTitle(title);
            // Use custom style with less rounded corners
            setStyle(createCustomNodeStyle(IM_COL32(71, 142, 173, 255)));
            
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
            setStyle(createCustomNodeStyle(IM_COL32(173, 126, 71, 255)));
            
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
            setStyle(createCustomNodeStyle(IM_COL32(142, 68, 173, 255)));
            
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
    

class CommentNode : public ImFlow::BaseNode {
public:

    CommentNode() {
        setTitle("Comment");
        setStyle(createCustomNodeStyle(IM_COL32(200, 160, 60, 255), "Comment"));
        
        m_commentText = "Double click to edit comment...";
        m_isEditing = false;
        
        // Initialize colors
        m_headerColor = IM_COL32(200, 160, 60, 180);
        m_bgColor = IM_COL32(200, 160, 60, 50);
        m_borderColor = IM_COL32(200, 160, 60, 200);
        
        // Set initial size
        m_customSize = ImVec2(200, 100);
        m_padding = 20.0f;
    }
    

    void draw() override { /* No content in the main body */ }
    

    bool usesCustomDrawing() const override { return true; }
    
    
    bool usesCustomHover() const override { return true; }
    

    bool customIsHovered() const override {
        auto handler = getHandler();
        if (!handler) return false;
        
        ImVec2 pos = getPos();
        ImVec2 size = getSize();
        
        return ImGui::IsMouseHoveringRect(handler->grid2screen(pos), handler->grid2screen(pos + size));
    }
    

    void updateCommentBounds() {

        if (m_containedNodes.empty()) {
            // If no contained nodes, use default size
            m_customSize = ImVec2(200, 100);
            return;
        }
        
        // Calculate bounds that encompass all contained nodes
        float minX = FLT_MAX, minY = FLT_MAX;
        float maxX = -FLT_MAX, maxY = -FLT_MAX;
        
        auto handler = getHandler();
        if (!handler) return;
        
        for (auto nodeId : m_containedNodes) {
            auto& nodes = handler->getNodes();
            auto it = nodes.find(nodeId);
            if (it != nodes.end()) {
                auto node = it->second;
                ImVec2 nodePos = node->getPos();
                ImVec2 nodeSize = node->getFullSize();
                
                minX = std::min(minX, nodePos.x);
                minY = std::min(minY, nodePos.y);
                maxX = std::max(maxX, nodePos.x + nodeSize.x);
                maxY = std::max(maxY, nodePos.y + nodeSize.y);
            }
        }
        
        // Add padding around the contained nodes
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
    

    void addContainedNode(ImFlow::NodeUID nodeId) { m_containedNodes.insert(nodeId); }
    

    void removeContainedNode(ImFlow::NodeUID nodeId) { m_containedNodes.erase(nodeId); }
    

    void clearContainedNodes() { m_containedNodes.clear(); }
    

    const std::set<ImFlow::NodeUID>& getContainedNodes() const { return m_containedNodes; }
    

    void customDraw(ImDrawList* draw_list, const ImVec2& offset) override {
        
        auto handler = getHandler();
        if (!handler) return;
        
        updateCommentBounds();
        ImVec2 screenPos = handler->grid2screen(getPos());
        ImVec2 size = getSize();
        
        // Draw main comment box (semi-transparent background with border)
        ImU32 bgColor = m_bgColor;
        ImU32 borderColor = m_borderColor;
        ImU32 headerBgColor = m_headerColor;
        
        // Main box
        draw_list->AddRectFilled(screenPos, screenPos + size, bgColor, 2.0f);
        draw_list->AddRect(screenPos, screenPos + size, borderColor, 2.0f, 0, 1.5f);
        
        // Header box (for comment text)
        float headerHeight = 25.0f;
        ImVec2 headerSize = ImVec2(size.x, headerHeight);
        draw_list->AddRectFilled(screenPos, screenPos + headerSize, headerBgColor, 3.0f, ImDrawFlags_RoundCornersTop);
        
        // Draw comment text
        ImGui::SetCursorScreenPos(screenPos + ImVec2(5.0f, 5.0f));
        ImGui::PushItemWidth(size.x - 35.0f); // Leave space for settings button
        
        if (m_isEditing) {
            char buffer[256];
            strncpy(buffer, m_commentText.c_str(), sizeof(buffer));
            if (ImGui::InputText("##CommentText", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                m_commentText = buffer;
                m_isEditing = false;
            }
            // Stop editing if click outside
            if (ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered()) {
                m_isEditing = false;
            }
        } else {
            ImGui::TextUnformatted(m_commentText.c_str());
            // Double click to edit
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                m_isEditing = true;
            }
        }
        
        ImGui::PopItemWidth();
        
        // Draw settings button in top-right corner
        ImVec2 settingsButtonPos = screenPos + ImVec2(size.x - 25.0f, 5.0f);
        ImVec2 settingsButtonSize = ImVec2(20.0f, 15.0f);
        
        // Settings button
        ImU32 buttonColor = IM_COL32(255, 255, 255, 150);
        if (ImGui::IsMouseHoveringRect(settingsButtonPos, settingsButtonPos + settingsButtonSize)) {
            buttonColor = IM_COL32(255, 255, 255, 200);
        }
        
        // Draw gear icon (simplified as 3 dots)
        draw_list->AddCircleFilled(settingsButtonPos + ImVec2(5.0f, 3.0f), 1.5f, buttonColor);
        draw_list->AddCircleFilled(settingsButtonPos + ImVec2(10.0f, 3.0f), 1.5f, buttonColor);
        draw_list->AddCircleFilled(settingsButtonPos + ImVec2(15.0f, 3.0f), 1.5f, buttonColor);
        
        // Button interaction
        ImGui::SetCursorScreenPos(settingsButtonPos);
        ImGui::InvisibleButton("##SettingsButton", settingsButtonSize);
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
            m_showSettings = true;
        }
        
        // Settings window
        if (m_showSettings) {
            drawSettingsWindow(screenPos, size);
        }
    }
    
    void drawSettingsWindow(const ImVec2& screenPos, const ImVec2& size) {
        // Position the settings window near the comment
        ImVec2 windowPos = screenPos + ImVec2(size.x + 5.0f, 0.0f);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(250, 0), ImGuiCond_Appearing);
        
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(40, 40, 40, 255));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, IM_COL32(60, 60, 60, 255));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, IM_COL32(80, 80, 80, 255));
        
        if (ImGui::Begin("Comment Settings", &m_showSettings, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            // Color pickers
            ImGui::Text("Colors:");
            ImGui::Separator();
            
            // Convert ImU32 to ImVec4 for color picker
            ImVec4 headerColorVec = ImColor(m_headerColor);
            ImVec4 bgColorVec = ImColor(m_bgColor);
            ImVec4 borderColorVec = ImColor(m_borderColor);
            
            if (ImGui::ColorEdit4("Header Color", (float*)&headerColorVec, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview)) {
                m_headerColor = ImColor(headerColorVec);
            }
            
            if (ImGui::ColorEdit4("Background Color", (float*)&bgColorVec, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview)) {
                m_bgColor = ImColor(bgColorVec);
            }
            
            if (ImGui::ColorEdit4("Border Color", (float*)&borderColorVec, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview)) {
                m_borderColor = ImColor(borderColorVec);
            }
            
            ImGui::Spacing();
            ImGui::Text("Layout:");
            ImGui::Separator();
            
            // Padding slider
            if (ImGui::SliderFloat("Padding", &m_padding, 5.0f, 50.0f, "%.0f px")) {
                // Update bounds when padding changes
                updateCommentBounds();
            }
            
            ImGui::Spacing();
            
            // Reset to defaults button
            if (ImGui::Button("Reset to Defaults")) {
                resetToDefaults();
            }
            
            ImGui::SameLine();
            
            // Close button
            if (ImGui::Button("Close")) {
                m_showSettings = false;
            }
        }
        ImGui::End();
        
        ImGui::PopStyleColor(3);
        
        // Close settings if clicked outside
        if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && 
            !ImGui::IsMouseHoveringRect(screenPos, screenPos + size)) {
            m_showSettings = false;
        }
    }
    
    void resetToDefaults() {
        m_headerColor = IM_COL32(200, 160, 60, 180);
        m_bgColor = IM_COL32(200, 160, 60, 50);
        m_borderColor = IM_COL32(200, 160, 60, 200);
        m_padding = 20.0f;
        updateCommentBounds();
    }
    

    void setCommentText(const std::string& text) { m_commentText = text; }
    
    
    const std::string& getCommentText() const { return m_commentText; }
    
    // Getter methods for settings (useful for serialization)
    ImU32 getHeaderColor() const { return m_headerColor; }
    ImU32 getBgColor() const { return m_bgColor; }
    ImU32 getBorderColor() const { return m_borderColor; }
    float getPadding() const { return m_padding; }
    
    // Setter methods for settings (useful for deserialization)
    void setHeaderColor(ImU32 color) { m_headerColor = color; }
    void setBgColor(ImU32 color) { m_bgColor = color; }
    void setBorderColor(ImU32 color) { m_borderColor = color; }
    void setPadding(float padding) { m_padding = padding; updateCommentBounds(); }
    
private:
    std::set<ImFlow::NodeUID> m_containedNodes;
    std::string m_commentText;
    bool m_isEditing = false;
    bool m_showSettings = false;
    ImVec2 m_customSize;  // Custom size for comment node
    
    // Customizable properties
    ImU32 m_headerColor;
    ImU32 m_bgColor;
    ImU32 m_borderColor;
    float m_padding;
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
    };
}
