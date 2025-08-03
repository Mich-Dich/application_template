
#include "util/pch.h"
#include "util/crash_handler.h"
#include "application.h"


#if defined(PLATFORM_LINUX)

    int main(int argc, char* argv[]) {

        AT::attach_crash_handler();
        
        AT::application app = AT::application(argc, argv);
        app.run();
        return EXIT_SUCCESS;
    }

#elif defined(PLATFORM_WINDOWS)

    #include <Windows.h>

    int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
        
        AT::attach_crash_handler();
        
        // Convert Windows command line to argc/argv
        int argc;
        wchar_t** argvw = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!argvw)                             // Handle error if conversion fails
            return EXIT_FAILURE;
        

        // Convert wide char arguments to UTF-8
        char** argv = new char*[argc];
        for (int i = 0; i < argc; ++i) {
            int size = WideCharToMultiByte(CP_UTF8, 0, argvw[i], -1, nullptr, 0, nullptr, nullptr);
            argv[i] = new char[size];
            WideCharToMultiByte(CP_UTF8, 0, argvw[i], -1, argv[i], size, nullptr, nullptr);
        }
        
        // Create and run application
        AT::application app(argc, argv);
        app.run();
        
        // Cleanup allocated memory
        LocalFree(argvw);
        for (int i = 0; i < argc; ++i) {
            delete[] argv[i];
        }
        delete[] argv;
        return EXIT_SUCCESS;
    }

#endif
