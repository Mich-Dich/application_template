
#include "../src/plugins/plugin_interface.h"
#include <iostream>

class test_plugin : public AT::plugin {
public:

    NAME(test_plugin)

    void execute() override {
        std::cout << "Hello from externally compiled plugin!\n";
    }
};

FACTORY_FUNC(test_plugin)




/*

# Compile plugin to shared library
g++ -std=c++23 -fPIC -shared example_plugin.cpp -o libtest_plugin.so

# Or with debugging symbols
g++ -std=c++23 -fPIC -shared -g example_plugin.cpp -o libtest_plugin.so

# Or with optimizations
g++ -std=c++23 -fPIC -shared -O2 example_plugin.cpp -o libtest_plugin.so

*/
