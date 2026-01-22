#pragma once

#include "ui/util/editor_window.h"
#include "ansi_parser.h"
#include <deque>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>

namespace GLT::UI {

    class log_window : public editor_window {
    public:
        log_window();
        virtual ~log_window() = default;

        void window() override;
        void update(const f32 delta_time) override;
        void on_event(GLT::event& event) override;

        // Thread-safe log methods
        void add_log(const std::string& message);
        void add_log(const char* message);
        void add_log_fmt(const char* fmt, ...);

        void clear();
        void set_max_lines(size_t max_lines);
        void set_auto_scroll(bool auto_scroll) { m_auto_scroll = auto_scroll; }
        void set_show_timestamp(bool show) { m_show_timestamp = show; }

        // Performance monitoring
        size_t get_line_count() const { return m_lines.size(); }
        size_t get_rendered_segments() const { return m_total_segments; }

    protected:
        virtual void show_possible_sub_window_options() override;

    private:
        struct cached_line {
            std::string raw_text;  // Original text with ANSI codes
            std::vector<std::pair<std::string, ImVec4>> segments;
            size_t approximate_size = 0;
            double timestamp = 0.0;

            bool visible = true;

            // Cached version of rendered segments (reused if text hasn't changed)
            mutable std::vector<std::pair<std::string, ImVec4>> cached_segments;
            mutable bool needs_parsing = true;

            void parse(AT::ansi_parser& parser);
        };

        std::deque<cached_line> m_lines;
        std::deque<cached_line> m_pending_lines;  // Thread-safe buffer
        AT::ansi_parser m_parser;

        std::mutex m_mutex;
        std::atomic<size_t> m_max_lines{10000};
        std::atomic<bool> m_auto_scroll{true};
        std::atomic<bool> m_show_timestamp{false};
        std::atomic<bool> m_paused{false};

        // Filtering
        char m_filter_buffer[256] = "";
        std::string m_filter;
        bool m_filter_dirty = true;

        // Performance
        mutable size_t m_total_segments = 0;
        double m_last_render_time = 0.0;
        size_t m_frames_since_last_parse = 0;

        // Options
        bool m_show_options = false;
        bool m_word_wrap = false;
        bool m_show_colors = true;
        ImVec4 m_default_color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

        // Rendering state
        bool m_scroll_to_bottom = false;
        ImGuiTextFilter m_text_filter;

        // UI helpers
        void render_toolbar();
        void render_log_content();
        void render_line(const cached_line& line);
        void process_pending_lines();
        void apply_filter();

        // Performance optimization
        mutable std::vector<size_t> m_visible_line_indices;
        bool m_visible_indices_dirty = true;

        // File operations
        void export_to_file();
        void copy_to_clipboard();
    };

}
