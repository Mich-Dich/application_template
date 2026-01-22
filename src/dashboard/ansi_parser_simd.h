#pragma once

#include <immintrin.h> // For SSE/AVX if available
#include "ansi_parser.h"


namespace AT {


    class simd_ansi_parser {
    public:

        // Use SIMD to quickly find escape sequences
        static size_t find_next_escape(const char* str, size_t len) {
            // Simple byte scan - in practice you'd use SIMD for this
            for (size_t i = 0; i < len; i++) {
                if (str[i] == '\x1b' && i + 1 < len && str[i + 1] == '[') {
                    return i;
                }
            }
            return len;
        }

        // Batch parse multiple lines
        template<typename Callback>
        void parse_batch(const std::vector<std::string>& lines, Callback&& callback) {

            for (const auto& line : lines) {
                parse_line_fast(line, callback);
            }
        }

        template<typename Callback>
        void parse_line_fast(const std::string& line, Callback&& callback) {

            const char* p = line.data();
            const char* end = p + line.size();
            const char* segment_start = p;
            ansi_color current_color = ansi_color::normal();

            while (p < end) {
                // Fast path for non-escape characters
                if (*p != '\x1b') {
                    p++;
                    continue;
                }

                // Check for escape sequence
                if (p + 1 < end && p[1] == '[') {
                    // Output current segment
                    if (p > segment_start) {
                        callback(segment_start, p - segment_start, current_color.to_ImVec4());
                    }

                    // Parse ANSI code
                    p += 2;
                    const char* code_start = p;

                    // Fast forward to 'm'
                    while (p < end && *p != 'm') p++;

                    if (p < end) {
                        process_ansi_code_fast(code_start, p - code_start, current_color);
                        p++; // Skip 'm'
                    }

                    segment_start = p;
                } else {
                    p++;
                }
            }

            // Final segment
            if (p > segment_start) {
                callback(segment_start, p - segment_start, current_color.to_ImVec4());
            }
        }

    private:
        void process_ansi_code_fast(const char* code, size_t len, ansi_color& current_color) {
            int value = 0;

            for (size_t i = 0; i < len; i++) {
                char c = code[i];

                if (c >= '0' && c <= '9') {
                    value = value * 10 + (c - '0');
                } else if (c == ';' || i == len - 1) {
                    apply_ansi_code_fast(value, current_color);
                    value = 0;
                }
            }
        }

        void apply_ansi_code_fast(int code, ansi_color& color) {
            // Use lookup table for speed
            static const ansi_color color_table[] = {
                ansi_color::normal(),    // 0
                ansi_color::normal(),    // 1 (placeholder for bold)
                ansi_color::normal(),    // 2
                ansi_color::normal(),    // 3
                ansi_color::normal(),    // 4
                ansi_color::normal(),    // 5
                ansi_color::normal(),    // 6
                ansi_color::normal(),    // 7
                ansi_color::normal(),    // 8
                ansi_color::normal(),    // 9
                ansi_color::normal(),    // 10
                // ... fill with appropriate colors
            };

            if (code >= 0 && code < 100) {
                // Handle common codes with table lookup
            } else {
                // Handle extended codes
            }
        }
    };

}