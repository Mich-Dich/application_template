
#include "util/pch.h"
#include "util/crash_handler.h"
#include "application.h"

int main(int argc, char* argv[]) {

    AT::attach_crash_handler();
    
    AT::application app = AT::application(argc, argv);
    app.run();
    return EXIT_SUCCESS;
}
