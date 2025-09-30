#pragma once

// Forward declarations to avoid direct dependencies
namespace AT {
    namespace logger {
        enum class severity { Trace, Debug, Info, Warn, Error, Fatal };
        
        // Function pointers that will be set by the main application
        extern void (*log_msg)(severity level, const char* file, const char* function, int line, const char* message);
        
        // Helper macro for plugins
        #define PLUGIN_LOG(level, message) \
            do { \
                if (AT::logger::log_msg) { \
                    AT::logger::log_msg(level, __FILE__, __FUNCTION__, __LINE__, message); \
                } \
            } while(0)
    }
}
