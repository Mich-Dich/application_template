// plugin_interface.h - WORKING VERSION
#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <any>
#include <vector>

#ifndef PLUGIN_API
    #ifdef _WIN32
        #define PLUGIN_API extern "C" __declspec(dllexport)
    #else
        #define PLUGIN_API extern "C"
    #endif
#endif

#define NAME(plugin_name)               std::string get_name() const override { return #plugin_name; }

#define FACTORY_FUNC(plugin_name)       PLUGIN_API AT::plugin* create_plugin()               { return new plugin_name(); }       \
                                        PLUGIN_API void destroy_plugin(AT::plugin* plugin)   { delete plugin; }


// defines for parser
#define PLUGIN_FUNC(...)


namespace AT {
    
    class plugin {
    public:
        virtual ~plugin() = default;
        
        // Base functionality
        virtual std::string get_name() const = 0;
        virtual void execute() = 0;
        
        // Dynamic function system
        virtual void register_functions() = 0;
        
        // Function management - simple and working
        template<typename Function>
        bool register_function(const std::string& name, Function func) {
            m_functions[name] = [func](const std::vector<std::any>& args) -> std::any {
                return call_function(func, args);
            };
            return true;
        }
        
        bool has_function(const std::string& name) const {
            return m_functions.find(name) != m_functions.end();
        }
        
        template<typename T, typename... Args>
        T call_function(const std::string& name, Args&&... args) {
            auto it = m_functions.find(name);
            if (it == m_functions.end()) {
                throw std::runtime_error("Function '" + name + "' not found in plugin");
            }
            
            std::vector<std::any> arg_list;
            (arg_list.push_back(std::any(std::forward<Args>(args))), ...);
            
            std::any result = it->second(arg_list);
            
            if constexpr (!std::is_void_v<T>) {
                try {
                    return std::any_cast<T>(result);
                } catch (const std::bad_any_cast& e) {
                    throw std::runtime_error(std::string("Return type mismatch for function '") + name + "': " + e.what());
                }
            }
        }
        
        template<typename... Args>
        void call_function_void(const std::string& name, Args&&... args) {
            call_function<void, Args...>(name, std::forward<Args>(args)...);
        }

    private:
        // Helper to call functions with proper argument unpacking
        template<typename Function>
        static auto call_function(Function func, const std::vector<std::any>& args) {
            return call_function_impl(func, args, std::make_index_sequence<std::tuple_size_v<function_args_t<Function>>>{});
        }
        
        template<typename Function, size_t... I>
        static auto call_function_impl(Function func, const std::vector<std::any>& args, std::index_sequence<I...>) {
            using result_type = function_return_t<Function>;
            
            if constexpr (std::is_void_v<result_type>) {
                func(std::any_cast<std::tuple_element_t<I, function_args_t<Function>>>(args[I])...);
                return std::any{};
            } else {
                return std::any(func(std::any_cast<std::tuple_element_t<I, function_args_t<Function>>>(args[I])...));
            }
        }
        
        // Type traits
        template<typename T>
        struct function_traits : function_traits<decltype(&T::operator())> {};
        
        template<typename R, typename... Args>
        struct function_traits<R(*)(Args...)> {
            using return_type = R;
            using args_type = std::tuple<Args...>;
        };
        
        template<typename R, typename C, typename... Args>
        struct function_traits<R(C::*)(Args...)> {
            using return_type = R;
            using args_type = std::tuple<Args...>;
        };
        
        template<typename R, typename C, typename... Args>
        struct function_traits<R(C::*)(Args...) const> {
            using return_type = R;
            using args_type = std::tuple<Args...>;
        };
        
        template<typename Function>
        using function_return_t = typename function_traits<Function>::return_type;
        
        template<typename Function>
        using function_args_t = typename function_traits<Function>::args_type;

    protected:
        std::unordered_map<std::string, std::function<std::any(const std::vector<std::any>&)>> m_functions;
    };

    using create_plugin_func = plugin* (*)();
    using destroy_plugin_func = void (*)(plugin*);
}
