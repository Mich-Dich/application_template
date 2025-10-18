#pragma once

#include <ImNodeFlow.h>
#include "util/ui/panel_collection.h"
#include "util/util.h"

#include "visual_programming/nodes/comment_node.h"
#include "visual_programming/nodes/math_expression_node.h"
#include "visual_programming/nodes/function_node.h"
#include "visual_programming/nodes/function_registry.h"

namespace AT {

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
                
        {"Math Expression", "Math Operations", 
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<math_expression_node>(pos); },
            "Dynamic math expression with automatic pin creation"},
            
        {"Plotter", "Visualization", 
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<PlotterNode>(pos); },
            "Create various types of plots and charts"},

        {"Comment", "Organization",
            [](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { editor.addNode<comment_node>(pos); },
            "Group nodes with a comment box"},
    };


    // Helper to create function nodes
    static void createFunctionNode(ImFlow::ImNodeFlow& editor, const ImVec2& pos, const std::string& functionName) {
        const auto* def = FunctionRegistry::getInstance().getFunction(functionName);
        if (def) {
            auto node = editor.addNode<FunctionNode>(pos, *def);
        }
    }


    // Create node definitions for all registered functions
    static std::vector<NodeDefinition> createFunctionNodeList() {
        std::vector<NodeDefinition> functionNodes;
        const auto& allFunctions = FunctionRegistry::getInstance().getAllFunctions();
        
        for (const auto& [name, def] : allFunctions) {
            functionNodes.push_back({
                def.title.c_str(),
                def.category.c_str(),
                [name](ImFlow::ImNodeFlow& editor, const ImVec2& pos) { 
                    createFunctionNode(editor, pos, name); 
                },
                def.description.c_str()
            });
        }
        
        return functionNodes;
    }


    // Combine existing nodes with function nodes
    static std::vector<NodeDefinition> getCompleteNodeList() {
        auto completeList = node_list;
        auto functionList = createFunctionNodeList();
        completeList.insert(completeList.end(), functionList.begin(), functionList.end());
        return completeList;
    }

}
