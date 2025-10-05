#pragma once

#include "plugin_interface.h"
#include "plugin_manager.h"

namespace AT::plugins {

    // Base class for all plugin interface wrappers
    class plugin_interface_base {
    public:

    #if defined(PLATFORM_LINUX)
        #define DEFAULT_PLUGIN_PATH(plugin_name)                                                            \
            (util::get_executable_path() / "plugins" / #plugin_name / ("lib" + std::string(#plugin_name) + ".so"))
    #elif defined(PLATFORM_WINDOWS)
        #define DEFAULT_PLUGIN_PATH(plugin_name)                                                            \
            (util::get_executable_path() / "plugins" / #plugin_name / (std::string(#plugin_name) + ".dll"))
    #endif

    #define DEFAULT_LOAD_PLUGIN(plugin_name)                                                                \
        static bool load_plugin(const std::filesystem::path& path = DEFAULT_PLUGIN_PATH(plugin_name)) {     \
            try {                                                                                           \
                get_plugin_manager().load_plugin(path);                                                     \
                return true;                                                                                \
            } catch (const std::exception& e) {                                                             \
                LOG(Error, "Failed to load [" << #plugin_name << "]: " << e.what());                        \
                return false;                                                                               \
            }                                                                                               \
        }                                                                                                   \
        static bool is_loaded()     { return get_plugin(#plugin_name) != nullptr; }

    protected:

        static plugin_manager& get_plugin_manager() {
            static plugin_manager instance;
            return instance;
        }
        
        static std::shared_ptr<plugin> get_plugin(const std::string& name) { return get_plugin_manager().get_plugin(name); }
        
        template<typename T, typename... Args>
        static T call_plugin_function(const std::string& plugin_name, const std::string& function_name, Args&&... args) {
            return get_plugin_manager().call_plugin_function<T>(
                plugin_name, function_name, std::forward<Args>(args)...
            );
        }

    };

}
