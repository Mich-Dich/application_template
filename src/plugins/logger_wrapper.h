// logger_wrapper.h - Updated version
#pragma once

#include "logger_interface.h"
#include "util/io/logger.h"

namespace AT {

    // Global pointer for plugin logger access
    extern logger_interface* g_plugin_logger;

    class logger_wrapper : public logger_interface {
    public:

        logger_wrapper() {
            // Set the global instance when this wrapper is created
            g_plugin_logger = this;
        }
        
        ~logger_wrapper() {
            // Clear the global instance when this wrapper is destroyed
            g_plugin_logger = nullptr;
        }
        
        void log_forward(const logger::severity msg_sev, const char* file_name, const char* function_name, const int line, const std::ostringstream& message) override {
            AT::logger::log_msg(msg_sev, file_name, function_name, line, std::this_thread::get_id(), std::move(message.str()));
        }

    };

}
