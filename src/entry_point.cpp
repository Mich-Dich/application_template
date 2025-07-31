
#include "application.h"
#include "util/crash_handler.h"

#if defined(PLATFORM_WINDOWS)

#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdLine, int cmdshow) {

// #if DEBUG                                       // open Konsole when in debug
//     AT::util::open_console("Gluttony Engine - Debug Console", true);
// #endif

//     AT::attach_crash_handler();                // catch any crashes from here on (dont need to call "AT::detach_crash_handler();" because crash hander is needed for the entire application)
//     auto app = AT::create_application(argc, argv);
//     app->run();
//     delete app;

    return EXIT_SUCCESS;
}

#elif defined(PLATFORM_LINUX)                   // dont need to create console for linux, because linux!

int main(int argc, char* argv[]) {

//     AT::attach_crash_handler();

//     AT::application* app = AT::create_application(argc, argv);
//     app->run();
//     delete app;
    
    return EXIT_SUCCESS;
}

#endif
