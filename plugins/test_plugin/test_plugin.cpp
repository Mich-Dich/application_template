
#include "util/pch.h"

#include "plugins/plugin_interface.h"

class test_plugin : public AT::plugin {
public:

    NAME(test_plugin)

    void execute() override {
        std::cout << "Hello from externally compiled plugin!\n";
    }
    
    void register_functions() override {            // Register custom functions - use explicit std::function

        register_function("calc_size", std::function<float()>(
            [this]() { return calc_size(); }
        ));
        
        register_function("process_data", std::function<std::string(const std::string&, int)>(
            [this](const std::string& data, int factor) { return process_data(data, factor); }
        ));
        
        register_function("initialize", std::function<void(float, bool)>(
            [this](float value, bool enabled) { initialize(value, enabled); }
        ));
    }


private:

    float calc_size() {
        // LOG(Trace, "Calculating size...")
        return 42.5f;
    }
    

    std::string process_data(const std::string& data, int factor) {
        std::string result;
        for (char c : data) {
            result += static_cast<char>(c + factor);
        }
        return result;
    }
    

    void initialize(float value, bool enabled) {
        m_initialized = true;
    }
    
    bool m_initialized = false;
};

FACTORY_FUNC(test_plugin)



/*

# with debugging symbols
g++ -std=c++23 -fPIC -shared -g example_plugin.cpp -o libtest_plugin.so

# or with optimizations
g++ -std=c++23 -fPIC -shared -O2 example_plugin.cpp -o libtest_plugin.so

*/
