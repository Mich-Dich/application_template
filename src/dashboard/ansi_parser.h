#pragma once

#include "util/pch.h"
#include <imgui.h>


namespace AT
{

    struct ansi_color {
        uint8_t r, g, b;
        bool bright;

        static constexpr ansi_color normal()            { return {192, 192, 192, false}; } // Light gray
        static constexpr ansi_color black()             { return {0, 0, 0, false}; }
        static constexpr ansi_color red()               { return {205, 0, 0, false}; }
        static constexpr ansi_color green()             { return {0, 205, 0, false}; }
        static constexpr ansi_color Yellow()            { return {205, 205, 0, false}; }
        static constexpr ansi_color Blue()              { return {0, 0, 238, false}; }
        static constexpr ansi_color Magenta()           { return {205, 0, 205, false}; }
        static constexpr ansi_color Cyan()              { return {0, 205, 205, false}; }
        static constexpr ansi_color White()             { return {229, 229, 229, false}; }

        static constexpr ansi_color bright_red()        { return {255, 0, 0, true}; }
        static constexpr ansi_color bright_green()      { return {0, 255, 0, true}; }
        static constexpr ansi_color bright_yellow()     { return {255, 255, 0, true}; }
        static constexpr ansi_color bright_blue()       { return {92, 92, 255, true}; }
        static constexpr ansi_color bright_magenta()    { return {255, 0, 255, true}; }
        static constexpr ansi_color bright_cyan()       { return {0, 255, 255, true}; }
        static constexpr ansi_color bright_white()      { return {255, 255, 255, true}; }

        ImVec4 to_ImVec4() const {
            float factor = bright ? 1.0f : 0.8f;
            return ImVec4(r/255.0f * factor, g/255.0f * factor, b/255.0f * factor, 1.0f);
        }
    };

    struct ansi_parser {

        struct Segment {
            const char*     text;
            size_t          length;
            ImVec4          color;
        };

        // State for parsing
        ansi_color          current_color = ansi_color::normal();
        bool                in_escape = false;
        std::string         escape_buffer;

        void reset() {

            current_color = ansi_color::normal();
            in_escape = false;
            escape_buffer.clear();
        }

        // Parse ANSI codes and return segments (zero-copy where possible)
        template<typename Callback>
        void parse(const char* text, size_t length, Callback&& callback) {

            const char* segment_start = text;
            const char* p = text;
            const char* end = p + length;

            while (p < end) {
                if (*p == '\x1b' && p + 1 < end && *(p + 1) == '[') {
                    // Output current segment if any
                    if (p > segment_start) {
                        callback(segment_start, p - segment_start, current_color.to_ImVec4());
                    }

                    // Parse escape sequence
                    p += 2; // Skip "\x1b["
                    const char* code_start = p;

                    while (p < end && (*p >= '0' && *p <= '9' || *p == ';')) {
                        p++;
                    }

                    if (p < end && *p == 'm') {
                        // Process ANSI code
                        process_ansi_code(code_start, p - code_start);
                        p++; // Skip 'm'
                    }

                    segment_start = p;
                } else {
                    p++;
                }
            }

            // Output remaining text
            if (p > segment_start) {
                callback(segment_start, p - segment_start, current_color.to_ImVec4());
            }
        }

    private:

        void process_ansi_code(const char* code, size_t length) {
            int value = 0;

            for (size_t i = 0; i < length; i++) {
                char c = code[i];

                if (c >= '0' && c <= '9') {
                    value = value * 10 + (c - '0');
                }

                if (c == ';' || i == length - 1) {
                    apply_ansi_code(value);
                    value = 0;
                }
            }
        }

        void apply_ansi_code(int code) {
            switch (code) {
                case 0:  current_color = ansi_color::normal(); break;
                case 1:  current_color.bright = true; break;
                case 22: current_color.bright = false; break;
                case 30: current_color = ansi_color::black(); break;
                case 31: current_color = ansi_color::red(); break;
                case 32: current_color = ansi_color::green(); break;
                case 33: current_color = ansi_color::Yellow(); break;
                case 34: current_color = ansi_color::Blue(); break;
                case 35: current_color = ansi_color::Magenta(); break;
                case 36: current_color = ansi_color::Cyan(); break;
                case 37: current_color = ansi_color::White(); break;
                case 90: current_color = ansi_color::black(); current_color.bright = true; break;
                case 91: current_color = ansi_color::bright_red(); break;
                case 92: current_color = ansi_color::bright_green(); break;
                case 93: current_color = ansi_color::bright_yellow(); break;
                case 94: current_color = ansi_color::bright_blue(); break;
                case 95: current_color = ansi_color::bright_magenta(); break;
                case 96: current_color = ansi_color::bright_cyan(); break;
                case 97: current_color = ansi_color::bright_white(); break;
            }
        }
    };

}
