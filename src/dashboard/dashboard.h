
#pragma once


namespace AT {

    class event;

    // =======================================================================
    //  dashboard
    // =======================================================================
    // The dashboard class represents the central UI and control layer of the
    // application. It manages initialization, rendering, updates, event
    // handling, and crash visualization. Users will mainly interact with this
    // class to control application state and customize its UI.
    //
    // Internally, it integrates with ImGui to create dockable panels, manage
    // windows, and provide runtime visualization tools.
    // =======================================================================
    class dashboard {
    public:

        // Constructs the dashboard object.
        // Sets up internal data but does not initialize ImGui or UI systems.
        dashboard();

        // Destroys the dashboard object.
        // Ensures all allocated resources are released if not already shut down.
        ~dashboard();


        // Initializes the dashboard and prepares all UI-related systems.
        // This should be called once after all other systems are initialized.
        // Reads application configuration to determine if a "long startup process"
        // is required, simulating longer load times if enabled.
        //
        // @return True if initialization succeeds, false otherwise.
        bool init();


        // Updates the dashboard logic each frame.
        // Typically used to handle per-frame logic, animations, or internal states.
        //
        // @param delta_time Time elapsed since the last frame, in seconds.
        void update(f32 delta_time);


        // Draws the main dashboard UI each frame.
        // Responsible for rendering ImGui's main dockspace, panels, and demo windows.
        //
        // @param delta_time Time elapsed since the last frame, in seconds.
        void draw(f32 delta_time);


        // Shuts down the dashboard and releases all allocated resources.
        // Should be called before any major system deinitialization to ensure
        // graceful cleanup of ImGui and related UI state.
        //
        // @return True if shutdown completes successfully, false otherwise.
        bool shutdown();


        // Handles application crashes.
        // Called automatically when a crash is detected by the crash handler.
        // Displays a crash message and provides hooks for custom crash handling.
        //
        // @return None.
        void on_crash();


        // Processes incoming application events.
        // Events may include keyboard input, mouse actions, window resizing,
        // or custom user-defined events.
        //
        // @param event Reference to the event to process.
        // @return None.
        void on_event(event& event);


        // Draws a special initialization UI.
        // This is used when `dashboard::init()` takes a long time to complete.
        // Displays an "Initializing..." message scaled dynamically to fit the window.
        //
        // Requires `[long_startup_process]` to be set to `true` in
        // `config/app_settings.yml` for this UI to be relevant.
        //
        // @param delta_time Time elapsed since the last frame, in seconds.
        void draw_init_UI(f32 delta_time);

    private:

        // No private data members yet.
        // You may want to add: persistent UI state, ImGui configuration, cached panel data, ...
    };
}
