
#include <iostream>
#include "util/crash_handler.h"

int main(int argc, char* argv[]) {

    AT::attach_crash_handler();

    std::cout << "Hello World" << std::endl;
    return EXIT_SUCCESS;
}
