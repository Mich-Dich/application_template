#include "util/pch_editor.h"
#include "ui/util/log_window.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <imgui.h>
#include <imgui_internal.h>

namespace GLT::UI {

    log_window::log_window() {
        make_window_name("Log");
        m_text_filter = ImGuiTextFilter(m_filter_buffer, sizeof(m_filter_buffer));
    }

    void log_window::window() {
        if (!m_show_window) return;

        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(m_window_name.c_str(), &m_show_window)) {
            render_toolbar();

            ImGui::Separator();

            // Log content area
            const float footer_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
            if (ImGui::BeginChild("LogContent", ImVec2(0, -footer_height), false,
                                ImGuiWindowFlags_HorizontalScrollbar |
                                (m_word_wrap ? ImGuiWindowFlags_AlwaysAutoResize : 0))) {

                render_log_content();

                // Auto-scroll if needed
                if (m_auto_scroll && m_scroll_to_bottom && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) {
                    ImGui::SetScrollHereY(1.0f);
                    m_scroll_to_bottom = false;
                }
            }
            ImGui::EndChild();

            // Filter input at bottom
            ImGui::Separator();
            if (ImGui::InputText("Filter", m_filter_buffer, sizeof(m_filter_buffer),
                                ImGuiInputTextFlags_EnterReturnsTrue)) {
                m_filter = m_filter_buffer;
                m_filter_dirty = true;
                m_visible_indices_dirty = true;
            }

            ImGui::SameLine();
            ImGui::TextDisabled("(%zu lines)", m_lines.size());
        }
        ImGui::End();
    }

    void log_window::update(const f32 delta_time) {
        // Process pending lines from other threads
        process_pending_lines();

        // Update filter if needed
        if (m_filter_dirty) {
            apply_filter();
            m_filter_dirty = false;
        }

        // Performance: Only reparse visible lines occasionally
        m_frames_since_last_parse++;
        if (m_frames_since_last_parse > 60) { // Every 60 frames
            m_frames_since_last_parse = 0;
            // Could add smart parsing logic here
        }
    }

    void log_window::on_event(GLT::event& event) {
        // Handle events if needed
    }

    void log_window::add_log(const std::string& message) {
        std::lock_guard<std::mutex> lock(m_mutex);

        cached_line new_line;
        new_line.raw_text = message;
        new_line.timestamp = ImGui::GetTime();
        new_line.needs_parsing = true;

        m_pending_lines.push_back(std::move(new_line));

        // Mark scroll for auto-scroll
        if (m_auto_scroll && !m_paused) {
            m_scroll_to_bottom = true;
        }
    }

    void log_window::add_log(const char* message) {
        add_log(std::string(message));
    }

    void log_window::add_log_fmt(const char* fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        add_log(buf);
    }

    void log_window::clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_lines.clear();
        m_pending_lines.clear();
        m_visible_line_indices.clear();
        m_total_segments = 0;
    }

    void log_window::set_max_lines(size_t max_lines) {
        m_max_lines = max_lines;

        std::lock_guard<std::mutex> lock(m_mutex);
        while (m_lines.size() > max_lines) {
            m_lines.pop_front();
        }
        m_visible_indices_dirty = true;
    }

    void log_window::process_pending_lines() {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_pending_lines.empty()) return;

        // Process all pending lines
        for (auto& line : m_pending_lines) {
            if (!m_filter.empty() && line.raw_text.find(m_filter) == std::string::npos) {
                line.visible = false;
            }

            m_lines.push_back(std::move(line));
        }

        // Trim to max lines
        while (m_lines.size() > m_max_lines) {
            m_lines.pop_front();
        }

        m_pending_lines.clear();
        m_visible_indices_dirty = true;
    }

    void log_window::render_toolbar() {
        // Options button
        if (ImGui::Button("Options")) {
            m_show_options = !m_show_options;
        }

        ImGui::SameLine();

        // Clear button
        if (ImGui::Button("Clear")) {
            clear();
        }

        ImGui::SameLine();

        // Pause toggle
        ImGui::PushStyleColor(ImGuiCol_Text, m_paused ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
        if (ImGui::Button(m_paused ? "Resume" : "Pause")) {
            m_paused = !m_paused;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Auto-scroll toggle
        ImGui::Checkbox("Auto-scroll", &m_auto_scroll);

        ImGui::SameLine();

        // Word wrap toggle
        ImGui::Checkbox("Word Wrap", &m_word_wrap);

        ImGui::SameLine();

        // Export button
        if (ImGui::Button("Export")) {
            export_to_file();
        }

        ImGui::SameLine();

        // Copy button
        if (ImGui::Button("Copy")) {
            copy_to_clipboard();
        }

        // Options panel
        if (m_show_options) {
            ImGui::Separator();
            ImGui::Checkbox("Show timestamps", &m_show_timestamp);
            ImGui::Checkbox("Show colors", &m_show_colors);

            ImGui::SliderInt("Max Lines", reinterpret_cast<int*>(&m_max_lines), 100, 100000);

            if (ImGui::ColorEdit3("Default Color", &m_default_color.x)) {
                // Update parser default color
                // You might need to modify ansi_parser to support custom default color
            }

            ImGui::Separator();
            ImGui::Text("Stats: %zu lines, %zu segments", m_lines.size(), m_total_segments);
        }
    }

    void log_window::render_log_content() {
        // Rebuild visible indices if needed
        if (m_visible_indices_dirty) {
            m_visible_line_indices.clear();
            for (size_t i = 0; i < m_lines.size(); ++i) {
                if (m_lines[i].visible) {
                    m_visible_line_indices.push_back(i);
                }
            }
            m_visible_indices_dirty = false;
        }

        // Use clipper for large logs
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(m_visible_line_indices.size()));

        m_total_segments = 0;

        while (clipper.Step()) {
            for (int line_idx = clipper.DisplayStart; line_idx < clipper.DisplayEnd; ++line_idx) {
                size_t actual_idx = m_visible_line_indices[line_idx];
                const auto& line = m_lines[actual_idx];

                // Parse line if needed
                if (line.needs_parsing) {
                    line.parse(m_parser);
                }

                render_line(line);
                m_total_segments += line.segments.size();
            }
        }
    }

    void log_window::render_line(const cached_line& line) {
        ImGui::PushID(static_cast<int>(line.timestamp * 1000));

        // Timestamp if enabled
        if (m_show_timestamp) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            char time_buf[32];
            std::time_t t = static_cast<std::time_t>(line.timestamp);
            std::strftime(time_buf, sizeof(time_buf), "%H:%M:%S", std::localtime(&t));
            ImGui::TextUnformatted(time_buf);
            ImGui::PopStyleColor();
            ImGui::SameLine(0, 10);
        }

        // Render colored segments
        if (m_show_colors && !line.segments.empty()) {
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
        } else {
            // Render as plain text (without colors)
            ImGui::TextUnformatted(line.raw_text.c_str());
        }

        ImGui::PopID();
    }

    void log_window::cached_line::parse(AT::ansi_parser& parser) {
        if (!needs_parsing) return;

        segments.clear();
        parser.parse(raw_text.c_str(), raw_text.size(),
            [this](const char* text, size_t len, ImVec4 color) {
                segments.emplace_back(std::string(text, len), color);
                approximate_size += len + sizeof(ImVec4);
            });

        needs_parsing = false;
        cached_segments = segments; // Cache for reuse
    }

    void log_window::apply_filter() {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_filter.empty()) {
            for (auto& line : m_lines) {
                line.visible = true;
            }
            for (auto& line : m_pending_lines) {
                line.visible = true;
            }
        } else {
            for (auto& line : m_lines) {
                line.visible = (line.raw_text.find(m_filter) != std::string::npos);
            }
            for (auto& line : m_pending_lines) {
                line.visible = (line.raw_text.find(m_filter) != std::string::npos);
            }
        }

        m_visible_indices_dirty = true;
    }

    void log_window::export_to_file() {
        // Simple file save dialog
        char filename[256] = "log_export.txt";

        if (ImGui::IsPopupOpen("Export Log")) {
            if (ImGui::BeginPopupModal("Export Log", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::InputText("Filename", filename, sizeof(filename));

                if (ImGui::Button("Save")) {
                    std::ofstream file(filename);
                    if (file.is_open()) {
                        for (const auto& line : m_lines) {
                            if (line.visible) {
                                // Strip ANSI codes for export
                                std::string clean_text;
                                m_parser.parse(line.raw_text.c_str(), line.raw_text.size(),
                                    [&clean_text](const char* text, size_t len, ImVec4) {
                                        clean_text.append(text, len);
                                    });
                                file << clean_text << "\n";
                            }
                        }
                        file.close();
                    }
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        } else {
            ImGui::OpenPopup("Export Log");
        }
    }

    void log_window::copy_to_clipboard() {
        std::string clipboard_text;

        for (const auto& line : m_lines) {
            if (line.visible) {
                // Strip ANSI codes for clipboard
                m_parser.parse(line.raw_text.c_str(), line.raw_text.size(),
                    [&clipboard_text](const char* text, size_t len, ImVec4) {
                        clipboard_text.append(text, len);
                    });
                clipboard_text += "\n";
            }
        }

        ImGui::SetClipboardText(clipboard_text.c_str());
    }

    void log_window::show_possible_sub_window_options() {
        // Add this window to a menu if needed
        if (ImGui::MenuItem("Log Window")) {
            m_show_window = true;
            focus_window();
        }
    }

}


/*

Performance Optimizations:

For even better performance with thousands of lines:
// Add to log_window.h
private:
    // Alternative: Use ring buffer for ultra-high performance
    template<typename T, size_t Capacity>
    class ring_buffer {
        std::array<T, Capacity> buffer;
        size_t head = 0;
        size_t tail = 0;
        size_t count = 0;

    public:
        void push_back(T&& item) { }
        T& operator[](size_t index) { }
        size_t size() const { return count; }
        void clear() { head = tail = count = 0; }
    };

    // Use fixed-size allocator for segments
    class segment_allocator {
        static constexpr size_t POOL_SIZE = 10000;
        struct segment_pool {
            char data[256];  // Fixed-size segments
            bool used = false;
        };
        std::vector<segment_pool> pool;

    public:
        char* allocate(size_t size) {
            // Find unused pool entry
            for (auto& entry : pool) {
                if (!entry.used && sizeof(entry.data) >= size) {
                    entry.used = true;
                    return entry.data;
                }
            }
            return nullptr;  // Fall back to heap
        }

        void deallocate(char* ptr) {
            // Mark as unused
            auto* entry = reinterpret_cast<segment_pool*>(
                reinterpret_cast<uintptr_t>(ptr) - offsetof(segment_pool, data));
            entry->used = false;
        }
    };

*/
