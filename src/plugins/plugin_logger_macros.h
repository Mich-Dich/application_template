#pragma once

#include "logger_interface.h"

// Forward declare the global plugin logger instance
namespace AT {
    extern logger_interface* g_plugin_logger;
}

#define PLUGIN_LOG_MAIN(log_severity, message)                                                                      \
    { std::ostringstream oss{}; oss << message;                                                                     \
        if (AT::g_plugin_logger) { AT::g_plugin_logger->log_forward(AT::logger::severity::log_severity, __FILE__,   \
            __FUNCTION__, __LINE__, oss); }                                                                         \
        else { std::cout << oss.str() << std::endl; }; }

// This enables the different log levels (FATAL + ERROR are always on)
//  0 = FATAL + ERROR
//  1 = FATAL + ERROR + WARN
//  2 = FATAL + ERROR + WARN + INFO
//  3 = FATAL + ERROR + WARN + INFO + DEBUG
//  4 = FATAL + ERROR + WARN + INFO + DEBUG + TRACE
#define PLUGIN_LOG_LEVEL_ENABLED           			4

#define PLUGIN_LOG_Fatal(message)           PLUGIN_LOG_MAIN(Fatal, message)
#define PLUGIN_LOG_Error(message)           PLUGIN_LOG_MAIN(Error, message)

#if PLUGIN_LOG_LEVEL_ENABLED > 0
    #define PLUGIN_LOG_Warn(message)        PLUGIN_LOG_MAIN(Warn, message)
#else
    #define PLUGIN_LOG_Warn(message)        { }
#endif

#if PLUGIN_LOG_LEVEL_ENABLED > 1
    #define PLUGIN_LOG_Info(message)        PLUGIN_LOG_MAIN(Info, message)
#else
    #define PLUGIN_LOG_Info(message)        { }
#endif

#if PLUGIN_LOG_LEVEL_ENABLED > 2
    #define PLUGIN_LOG_Debug(message)       PLUGIN_LOG_MAIN(Debug, message)
#else
    #define PLUGIN_LOG_Debug(message)       { }
#endif

#if PLUGIN_LOG_LEVEL_ENABLED > 3
    #define PLUGIN_LOG_Trace(message)       PLUGIN_LOG_MAIN(Trace, message)
#else
    #define PLUGIN_LOG_Trace(message)       { }
#endif



#ifdef LOG
    #undef LOG
#endif

// Shortcut macros matching original style
#define LOG(log_severity, message)          PLUGIN_LOG_##log_severity(message)



// Plugin assertion macros
#define PLUGIN_ASSERT(expr, message)                                                \
    if (!(expr)) {                                                                  \
        PLUGIN_LOG_Fatal("Assertion failed: " << #expr << " - " << message);        \
        throw std::runtime_error("Assertion failed: " + std::string(#expr));        \
    }

#define PLUGIN_ASSERT_S(expr)                                                       \
    if (!(expr)) {                                                                  \
        PLUGIN_LOG_Fatal("Assertion failed: " << #expr);                            \
        throw std::runtime_error("Assertion failed: " + std::string(#expr));        \
    }

// Plugin validation macros
#define PLUGIN_VALIDATE(expr, command, message_success, message_failure)            \
    if (expr) {                                                                     \
        PLUGIN_LOG_Trace(message_success);                                          \
    } else {                                                                        \
        PLUGIN_LOG_Error(message_failure);                                          \
        command;                                                                    \
    }

#define PLUGIN_VALIDATE_S(expr, command)                                            \
    if (!(expr)) {                                                                  \
        PLUGIN_LOG_Error("Validation failed: " << #expr);                           \
        command;                                                                    \
    }


#ifdef VALIDATE
    #undef VALIDATE
#endif
#ifdef VALIDATE_S
    #undef VALIDATE_S
#endif

#ifdef ASSERT
    #undef ASSERT
#endif
#ifdef ASSERT_S
    #undef ASSERT_S
#endif

// Shortcuts for validation and assertion
#define VALIDATE(expr, command, message_success, message_failure)   PLUGIN_VALIDATE(expr, command, message_success, message_failure)
#define VALIDATE_S(expr, command)                                   PLUGIN_VALIDATE_S(expr, command)
#define ASSERT(expr, message_success, message_failure)              PLUGIN_ASSERT(expr, message)
#define ASSERT_S(expr)                                              PLUGIN_ASSERT_S(expr)
