#pragma once

#include "util/pch.h"
#include <imgui.h>

#include "ansi_parser.h"


namespace AT {

    class colored_log {

        struct cached_line {
            std::vector<std::pair<std::string, ImVec4>>     segments;
            size_t                                          approximate_size; // For memory management
        };

        std::deque<cached_line>                             cached_lines;
        ansi_parser                                         parser;
        size_t                                              max_lines = 10000;
        bool                                                auto_scroll = true;

    public:

        void ddd_line(const std::string& line) {

            cached_line cached;
            parser.Parse(line.data(), line.size(), [&](const char* text, size_t len, ImVec4 color) {
                cached.segments.emplace_back(std::string(text, len), color);
                cached.approximate_size += len + sizeof(ImVec4);
            });

            cached_lines.push_back(std::move(cached));

            // Limit memory usage
            while (cached_lines.size() > max_lines) {
                cached_lines.pop_front();
            }
        }

        void ddd_line_fast(const char* line, size_t length) {

            cached_line cached;
            cached.segments.reserve(4); // Pre-allocate for typical cases

            parser.Parse(line, length, [&](const char* text, size_t len, ImVec4 color) {
                cached.segments.emplace_back(text, len);
                cached.segments.back().second = color;
            });

            cached_lines.push_back(std::move(cached));

            if (cached_lines.size() > max_lines) {
                cached_lines.pop_front();
            }
        }

        void draw(const char* title, bool* p_open = nullptr) {

            ImGui::Begin(title, p_open);

            // Optional: filter input
            static char filter[256] = "";
            ImGui::InputText("Filter", filter, sizeof(filter));

            // Display lines
            ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            for (const auto& line : cached_lines) {
                ImGui::BeginGroup();
                for (const auto& segment : line.segments) {
                    ImGui::PushStyleColor(ImGuiCol_Text, segment.second);
                    ImGui::TextUnformatted(segment.first.c_str());
                    ImGui::PopStyleColor();
                    if (&segment != &line.segments.back()) {
                        ImGui::SameLine(0, 0);
                    }
                }
                ImGui::EndGroup();
            }

            if (auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndChild();
            ImGui::End();
        }

        void clear() {

            cached_lines.clear();
        }

        void set_max_lines(size_t max) {

            max_lines = max;
            while (cached_lines.size() > max_lines) {
                cached_lines.pop_front();
            }
        }
    };

}
