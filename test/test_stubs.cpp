// test_stubs.cpp
#include <string>
#include <chrono>
#include <filesystem>

namespace AT::util {
    std::string get_system_time() {
        // Return a fixed string for testing
        return "00:00:00";
    }

    std::string get_executable_path() {
        return "/test/path";
    }

    std::string add_spaces(unsigned int count, unsigned int spaces_per_indent) {
        return std::string(count * spaces_per_indent, ' ');
    }

    unsigned int measure_indentation(const std::string& line, unsigned int spaces_per_indent) {
        unsigned int count = 0;
        while (count < line.size() && line[count] == ' ') {
            count++;
        }
        return count / spaces_per_indent;
    }
}