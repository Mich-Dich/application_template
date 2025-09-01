
#pragma once


namespace AT {

    class event;

    class dashboard {
    public:

        dashboard();
        ~dashboard();

        bool init();
        void update(f32 delta_time);
        void draw(f32 delta_time);
        bool shutdown();

        // invoked when a crash is detected by the crash_handler
        void on_crash();

        //
        void on_event(event& event);

        // used when dashboard::init takes some time. ! needs [long_startup_process] to be set to true in [config/app_settings.yml]
        void draw_init_UI(f32 delta_time);

    private:

    };
}
