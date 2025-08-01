
#include "util/crash_handler.h"
#include "application.h"

int main(int argc, char* argv[]) {

    AT::attach_crash_handler();
    
    AT::application app = AT::application(argc, argv);
    app.run();
    return EXIT_SUCCESS;
}


/*

mich:~/workspace/application_template$ xprop
_NET_WM_ICON_GEOMETRY(CARDINAL) = 555, 1032, 52, 44
_NET_WM_ALLOWED_ACTIONS(ATOM) = _NET_WM_ACTION_MOVE, _NET_WM_ACTION_RESIZE, _NET_WM_ACTION_MINIMIZE, _NET_WM_ACTION_MAXIMIZE_VERT, _NET_WM_ACTION_MAXIMIZE_HORZ, _NET_WM_ACTION_FULLSCREEN, _NET_WM_ACTION_CHANGE_DESKTOP, _NET_WM_ACTION_CLOSE
_NET_WM_DESKTOP(CARDINAL) = 1
_KDE_NET_WM_ACTIVITIES(STRING) = "5f4585f4-4d7e-4954-89df-0684e6560424"
WM_STATE(WM_STATE):
		window state: Normal
		icon window: 0x0
_NET_WM_STATE(ATOM) = 
_NET_WM_ICON_NAME(UTF8_STRING) = "ISW - Dear ImGui Demo"
_NET_WM_NAME(UTF8_STRING) = "ISW - Dear ImGui Demo"
WM_LOCALE_NAME(STRING) = "en_US.UTF-8"
WM_CLIENT_MACHINE(STRING) = "michtowerPC"
WM_ICON_NAME(STRING) = "ISW - Dear ImGui Demo"
WM_NAME(STRING) = "ISW - Dear ImGui Demo"
_KDE_NET_WM_USER_CREATION_TIME(CARDINAL) = 6995716
XdndAware(ATOM) = BITMAP
WM_CLASS(STRING) = "imgui_sub_window", "imgui_sub_window"
WM_NORMAL_HINTS(WM_SIZE_HINTS):
		program specified location: 0, 0
		window gravity: Static
WM_HINTS(WM_HINTS):
		Initial state is Normal State.
_NET_WM_WINDOW_TYPE(ATOM) = _NET_WM_WINDOW_TYPE_NORMAL
_NET_WM_PID(CARDINAL) = 25658
WM_PROTOCOLS(ATOM): protocols  WM_DELETE_WINDOW, _NET_WM_PING
_MOTIF_WM_HINTS(_MOTIF_WM_HINTS) = 0x2, 0x0, 0x0, 0x0, 0x0
mich:~/workspace/application_template$ 

*/