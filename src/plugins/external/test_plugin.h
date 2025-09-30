// plugins/test_plugin.h
#pragma once

#include "plugins/plugin_interface_wrapper.h"

namespace AT::plugins {

    class test_plugin : public plugin_interface_base {
    public:

        static float calc_size() {
            return call_plugin_function<float>("test_plugin", "calc_size");
        }
        
        static std::string process_data(const std::string& data, int factor) {
            return call_plugin_function<std::string>("test_plugin", "process_data", data, factor);
        }
        
        static void initialize(float value, bool enabled) {
            call_plugin_function<void>("test_plugin", "initialize", value, enabled);
        }
        
        // Optional: Add loading method
        static bool load_plugin(const std::filesystem::path& path = get_default_path()) {
            try {
                get_plugin_manager().load_plugin(path);
                return true;
            } catch (const std::exception& e) {
                LOG(Error, "Failed to load test_plugin: " << e.what());
                return false;
            }
        }
        
        static bool is_loaded() {
            return get_plugin("test_plugin") != nullptr;
        }

    private:
        static std::filesystem::path get_default_path() {
            return util::get_executable_path().parent_path().parent_path().parent_path() 
                / "plugins" / "libtest_plugin.so";
        }
    };

}
