// logger_interface.h
#pragma once

#include "util/pch.h"


namespace AT {
 
    class logger_interface {
    public:
 
       virtual ~logger_interface() = default;
        
        virtual void log_forward(const logger::severity msg_sev, const char* file_name, const char* function_name, const int line, const std::ostringstream& message) = 0;
    };

}
