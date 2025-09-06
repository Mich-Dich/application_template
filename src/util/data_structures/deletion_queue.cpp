
#include "util/pch.h"
#include "deletion_queue.h"


namespace AT::util {


    void deletion_queue::push_func(std::function<void()>&& function) {

        std::lock_guard<std::mutex> lock(m_mutex);
        m_functions.push_back(std::move(function));
    }


    void deletion_queue::flush() {

        std::vector<std::function<void()>> functions_to_execute;
        
        // Swap under lock to minimize time spent in critical section
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::swap(functions_to_execute, m_functions);
        }
        
        // Execute all functions, even if some throw exceptions
        std::exception_ptr first_exception;
        
        for (auto it = functions_to_execute.rbegin(); it != functions_to_execute.rend(); ++it) {
            try {
                if (*it) { // Check if function is valid
                    (*it)();
                }
            } catch (...) {
                // Store the first exception to rethrow later
                if (!first_exception) {
                    first_exception = std::current_exception();
                }
                // Continue executing remaining functions
            }
        }
        
        // If any function threw an exception, rethrow the first one
        if (first_exception) {
            std::rethrow_exception(first_exception);
        }
    }

}
