// plugins/plugin_interface_wrapper.h
#pragma once

#include "plugin_interface.h"
#include "plugin_manager.h"

namespace AT::plugins {

    // Base class for all plugin interface wrappers
    class plugin_interface_base {
    protected:

        static plugin_manager& get_plugin_manager() {
            static plugin_manager instance;
            return instance;
        }
        
        static std::shared_ptr<plugin> get_plugin(const std::string& name) {
            return get_plugin_manager().get_plugin(name);
        }
        
        template<typename T, typename... Args>
        static T call_plugin_function(const std::string& plugin_name, const std::string& function_name, Args&&... args) {
            return get_plugin_manager().call_plugin_function<T>(
                plugin_name, function_name, std::forward<Args>(args)...
            );
        }
    };

}
