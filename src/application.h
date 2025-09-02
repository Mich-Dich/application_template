
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
        DEFAULT_GETTER_C(f64, delta_time);

        // Returns a reference to the renderer responsible for drawing frames.
        // @return A reference-counted pointer to the renderer instance.
        DEFAULT_GETTER(ref<AT::render::renderer>, renderer);

        // Returns a reference to the application window.
        // @return A shared reference to the main window instance.
        DEFAULT_GETTER_S(ref<window>, window);

        // Returns a reference to the ImGui configuration object.
        // @return A reference to the ImGui configuration settings.
        DEFAULT_GETTER_REF(ref<UI::imgui_config>, imgui_config);

        // Returns a reference to the application's dashboard system.
        // @return A reference-counted pointer to the dashboard instance.
        DEFAULT_GETTER(ref<dashboard>, dashboard);


        // Provides global access to the current application instance.
        // @return A reference to the singleton application object.
        FORCEINLINE static application& get() { return *s_instance; }


        // Requests the application to stop running and exit its main loop.
        // @return None.
        FORCEINLINE static void close_application() { s_running = false; }


        // Updates application state for the current frame, processes logic,
        // and dispatches relevant events.
        // @param delta_time Time elapsed since the last frame.
        // @return True if the update succeeded, false otherwise.
        bool update(f32 delta_time);


        // Renders the current frame using the active renderer.
        // @param delta_time Time elapsed since the last frame.
        // @return True if the frame was drawn successfully, false otherwise.
        bool draw(f32 delta_time);


        // Starts the application’s main execution loop, initializes subsystems,
        // and processes events until the application is closed.
        // @return None.
        void run();

        // ---------------------- FPS Control ----------------------

        // Sets the desired target FPS for the application.
        // @param target_fps Desired frames per second.
        // @return None.
        void set_fps_settings(u32 target_fps);


        // Sets separate FPS limits depending on window focus.
        // @param set_for_engine_focused If true, applies [new_limit] to focused state;
        //        otherwise applies to unfocused state.
        // @param new_limit Desired frames per second.
        // @return None.
        void set_fps_settings(const bool set_for_engine_focused, const u32 new_limit);

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

        // ---------------------- FPS Management ----------------------

        // Starts measuring frame times for FPS calculations.
        // @return None.
        void start_fps_measurement();


        // Ends frame timing and calculates the duration of the current frame.
        // @param work_time Reference to store measured work time in seconds.
        // @return None.
        void end_fps_measurement(f32& work_time);


        // Limits FPS by sleeping the thread if frame computation finishes too early.
        // Updates delta time, absolute time, and current FPS counters.
        // @return None.
        void limit_fps();

        
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
        f32							        m_delta_time = 0.f;
        f32							        m_absolute_time = 0.f;
        f32							        m_work_time{}, m_sleep_time{};
        f32							        target_duration{};
        f32							        m_last_frame_time = 0.f;
    };

}
