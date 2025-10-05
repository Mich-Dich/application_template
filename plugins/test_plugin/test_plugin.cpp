#include "util/pch.h"
#include "plugins/plugin_interface.h"
#include "plugins/plugin_logger_macros.h"


class test_plugin : public AT::plugin {
public:

    NAME(test_plugin)

    void set_logger(AT::logger_interface* logger) override {
        m_logger = logger;
        AT::g_plugin_logger = logger;
    }

    void execute() override {

        LOG(Info, "Hello from externally compiled plugin!");

    }
    
    void register_functions() override {
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
    AT::logger_interface* m_logger = nullptr;

    float calc_size() {

        LOG(Trace, "Calculating size...");
        ASSERT_S(m_logger != nullptr);
        
        return 42.5f;
    }
    
    std::string process_data(const std::string& data, int factor) {

        VALIDATE(factor > 0, return "", "Processing data with valid factor", "Invalid factor provided");
        
        std::string result;
        for (char c : data) {
            result += static_cast<char>(c + factor);
        }
        
        LOG(Debug, "Processed data: " << data << " -> " << result);
        return result;
    }
    
    void initialize(float value, bool enabled) {

        VALIDATE(value >= 0.0f, return, "Initializing with valid value", "Invalid initialization value: " << value);
        
        m_initialized = true;
        LOG(Info, "Plugin initialized with value: " << value << ", enabled: " << enabled);
    }
    
    bool m_initialized = false;
};

FACTORY_FUNC(test_plugin)
