#pragma once

#include <string_view>
#include <string>
#include <vector>


#define API                             extern "C"

#define NAME(plugin_name)               std::string_view get_name() const override { return #plugin_name; }

#define FACTORY_FUNC(plugin_name)       API AT::plugin* create_plugin()               { return new plugin_name(); }       \
                                        API void destroy_plugin(AT::plugin* plugin)   { delete plugin; }


namespace AT {
    
    class plugin {
    public:

        virtual ~plugin() = default;
        virtual std::string_view get_name() const = 0;
        virtual void execute() = 0;
    };

    
    API {
        using create_plugin_func = plugin* (*)();
        using destroy_plugin_func = void (*)(plugin*);
    }

}
