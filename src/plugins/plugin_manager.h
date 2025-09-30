#pragma once 

#include "util/pch.h"
#include "plugins/plugin_interface.h"

namespace AT {

    class plugin_manager {
    public:
        ~plugin_manager();

        void load_plugin(const std::filesystem::path& path);
        
        // Plugin management
        std::shared_ptr<plugin> get_plugin(const std::string& name) const {
            auto it = m_plugins.find(name);
            return it != m_plugins.end() ? it->second : nullptr;
        }
        
        bool plugin_has_function(const std::string& plugin_name, const std::string& function_name) const {
            auto plugin = get_plugin(plugin_name);
            return plugin && plugin->has_function(function_name);
        }

        // Enhanced function calling (templates must be in header)
        template<typename T, typename... Args>
        T call_plugin_function(const std::string& plugin_name, const std::string& function_name, Args&&... args) {
            auto plugin_ptr = get_plugin(plugin_name);
            if (!plugin_ptr) {
                throw std::runtime_error("Plugin '" + plugin_name + "' not found");
            }
            return plugin_ptr->template call_function<T, Args...>(function_name, std::forward<Args>(args)...);
        }
        
        template<typename... Args>
        void call_plugin_function_void(const std::string& plugin_name, const std::string& function_name, Args&&... args) {
            call_plugin_function<void, Args...>(plugin_name, function_name, std::forward<Args>(args)...);
        }

    private:
        std::vector<void*> handles_;
        std::unordered_map<std::string, std::shared_ptr<plugin>> m_plugins;
    };
    
}