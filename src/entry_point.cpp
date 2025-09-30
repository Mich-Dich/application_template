
#include "util/pch.h"

#if 0

#include "util/timing/instrumentor.h"
#include "util/crash_handler.h"
#include "application.h"

#if defined(PLATFORM_LINUX)
    #define ARGC        argc
    #define ARGV        argv
    #define MAIN_FUNC   main(int argc, char* argv[])
#elif defined(PLATFORM_WINDOWS)
    #include <Windows.h>
    #define ARGC        __argc
    #define ARGV        __argv
    #define MAIN_FUNC   WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif



int MAIN_FUNC {
    
    PROFILER_SESSION_BEGIN("application", AT::util::get_executable_path() / "profiler", "application.json");
    {

        PROFILE_SCOPE("sub-systems startup");
        AT::crash_handler::attach();
        AT::logger::init("[$B$T:$J$E] [$B$L$X $Q - $I:$P:$G$E] $C$Z", true, AT::util::get_executable_path() / "logs", "application.log", true);
        AT::logger::set_buffer_threshold(AT::logger::severity::Warn);
        AT::logger::register_label_for_thread("main");
        AT::crash_handler::subscribe(AT::logger::shutdown);
    }

    {   // put application in scope to guarantee termination at specific point
        AT::application app = AT::application(ARGC, ARGV);
        app.run();
    }

    {
        PROFILE_SCOPE("sub-systems shutdown");
        
        AT::logger::shutdown();
        AT::crash_handler::detach();
    }

    PROFILER_SESSION_END();
    return EXIT_SUCCESS;
}

#else       // Test plugin system

#include <dlfcn.h>

#include "util/system.h"
#include "plugins/plugin_interface.h"

namespace AT {

    class plugin_manager {
        public:
        void load_plugin(const std::filesystem::path& path) {
            
            void* handle = dlopen(path.string().c_str(), RTLD_LAZY);        // Load shared library
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
                if (create_plugin_error) std::cout << "  create_plugin: " << create_plugin_error << std::endl;
                if (destroy_plugin_error) std::cout << "  destroy_plugin: " << destroy_plugin_error << std::endl;
                throw std::runtime_error("Failed to find plugin functions");
            }
            
            // Create plugin instance with custom deleter
            std::unique_ptr<plugin, void(*)(plugin*)> 
            plugin(create_plugin(), destroy_plugin);
            
            std::cout << "Loaded plugin: " << plugin->get_name() << "\n";
            plugin->execute();
            
            // Handle will be closed when unique_ptr goes out of scope
            // Store handle if you need to keep plugin loaded longer
            handles_.push_back(handle);
        }
        
        ~plugin_manager() {
            for (auto handle : handles_) {
                dlclose(handle);
            }
        }
        
        private:
        std::vector<void*> handles_;
    };
    
}


int main() {
    AT::plugin_manager manager;

    try {
        manager.load_plugin(AT::util::get_executable_path().parent_path().parent_path().parent_path() / "plugins" / "libtest_plugin.so");       // dont want to copy compiled plugin for every test
    } catch (const std::exception& e) {
        std::cout << "Failed to load plugin: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
#endif
