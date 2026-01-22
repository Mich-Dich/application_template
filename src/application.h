
#pragma once

#include "config/imgui_config.h"
#include "dashboard/dashboard.h"


namespace AT {

    class window;
    class event;
    class window_resize_event;
    class window_close_event;
    class window_refresh_event;
    class window_focus_event;
    class dashboard;
    namespace UI        { class imgui_config; }
    namespace render    { class renderer; }
    namespace util      { class fps_limiter; }

    class application {
    public:

        // Constructs the application object, initializes subsystems, creates the main window,
        // sets up the renderer, and configures ImGui, dashboard, and crash handling.
        // @param argc Number of command-line arguments.
        // @param argv Array of command-line argument strings.
        // @return None.
        application(int argc, char* argv[]);

        // Destroys the application instance, releasing all allocated resources,
        // shutting down subsystems, and cleaning up the renderer and window.
        // @return None.
        ~application();

        // Returns the delta time (time elapsed between the previous and current frame).
        // @return The time delta in seconds.
        DEFAULT_GETTER_C(f64,                           delta_time);

        // Returns a reference to the renderer responsible for drawing frames.
        // @return A reference-counted pointer to the renderer instance.
        DEFAULT_GETTER(ref<AT::render::renderer>,       renderer);

        // Returns a reference to the application window.
        // @return A shared reference to the main window instance.
        DEFAULT_GETTER_S(ref<window>,                   window);

        // Returns a reference to the ImGui configuration object.
        // @return A reference to the ImGui configuration settings.
        DEFAULT_GETTER_REF(ref<UI::imgui_config>,       imgui_config);

        // Returns a reference to the application's dashboard system.
        // @return A reference-counted pointer to the dashboard instance.
        DEFAULT_GETTER(ref<dashboard>,                  dashboard);


        DEFAULT_GETTER_REF(util::fps_limiter,           fps_limiter);


        // Provides global access to the current application instance.
        // @return A reference to the singleton application object.
        FORCEINLINE static application& get() { return *s_instance; }


        // Requests the application to stop running and exit its main loop.
        // @return None.
        FORCEINLINE static void close_application() { s_running = false; }


        // Starts the application’s main execution loop, initializes subsystems,
        // and processes events until the application is closed.
        // @return None.
        void run();

    protected:

        ref<UI::imgui_config>               m_imgui_config;     // Stores ImGui configuration settings.
        std::vector<event>                  m_event_queue;      // Event queue. TODO: Change to std::queue for FIFO.
        ref<AT::render::renderer>           m_renderer;         // Active rendering backend.

    private:

        // ---------------------- Event Handling ----------------------

        // Handles an incoming event by dispatching it to the appropriate handler.
        // @param event The event object to process.
        // @return None.
        void on_event(event& event);


        // Handles window close events and stops the main application loop.
        // @param event The window close event object.
        // @return Always returns true.
        bool on_window_close(window_close_event& event);


        // Handles window resize events by updating the renderer’s viewport size.
        // @param event The window resize event object.
        // @return Always returns true.
        bool on_window_resize(window_resize_event& event);


        // Handles window refresh events by redrawing the current frame.
        // @param event The window refresh event object.
        // @return Always returns true.
        bool on_window_refresh(window_refresh_event& event);


        // Handles window focus changes by adjusting FPS settings accordingly.
        // @param event The window focus event object.
        // @return Always returns true.
        bool on_window_focus(window_focus_event& event);


        static application*			        s_instance;
        static ref<window>		            s_window;
        static bool					        s_running;

        ref<dashboard>                      m_dashboard;
        u64                                 m_crash_subscription = 0;
        bool						        m_focus = true;
        bool                                m_is_titlebar_hovered = false;
        u32							        m_target_fps = 60;
        u32							        m_nonefocus_fps = 30;
        u32							        m_fps{};

        f32							        m_absolute_time = 0.f, m_delta_time = 0.f;
        util::fps_limiter                   m_fps_limiter;
    };

}
