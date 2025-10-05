#include "util/pch.h"
#include <dlfcn.h>

#include "logger_wrapper.h"
#include "plugin_manager.h"

namespace AT {

    // Make the logger_wrapper a static member or global so it persists
    static logger_wrapper g_logger_wrapper;

    plugin_manager::~plugin_manager() {
        for (auto handle : handles_) {
            dlclose(handle);
        }
    }

    void plugin_manager::load_plugin(const std::filesystem::path& path) {
        
        void* handle = dlopen(path.string().c_str(), RTLD_LAZY);
        if (!handle)
            throw std::runtime_error(dlerror());
        
        // Get factory functions
        auto create_plugin = reinterpret_cast<create_plugin_func>(dlsym(handle, "create_plugin"));
        auto create_plugin_error = dlerror();
        
        auto destroy_plugin = reinterpret_cast<destroy_plugin_func>(dlsym(handle, "destroy_plugin"));
        auto destroy_plugin_error = dlerror();
        
        if (!create_plugin || !destroy_plugin) {
            dlclose(handle);
            std::cout << "Symbol lookup errors:" << std::endl;
            if (create_plugin_error)    LOG(Error, "create_plugin: [" << create_plugin_error << "]");
            if (destroy_plugin_error)   LOG(Error, "destroy_plugin: [" << destroy_plugin_error << "]");
            throw std::runtime_error("Failed to find plugin functions");
        }
        
        // Create plugin instance with custom deleter
        std::shared_ptr<plugin> plugin_ptr(create_plugin(), [destroy_plugin](plugin* p) { destroy_plugin(p); });
        LOG(Trace, "Loaded plugin: " << plugin_ptr->get_name());

        plugin_ptr->set_logger(&g_logger_wrapper);
        plugin_ptr->register_functions();                   // Register plugin functions
        m_plugins[plugin_ptr->get_name()] = plugin_ptr;     // Store plugin and handle
        handles_.push_back(handle);
        
        plugin_ptr->execute();                              // Execute main function
    }
}