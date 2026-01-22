
#include "util/pch.h"

#include "util/timing/instrumentor.h"
#include "util/timing/timing_controller.h"
#include "util/crash_handler.h"
#include "util/util.h"
#include "platform/window.h"
#include "events/event.h"
#include "events/application_event.h"
#include "events/mouse_event.h"
#include "events/key_event.h"
#include "render/renderer.h"

#if defined(RENDER_API_OPENGL)
    #include "render/open_GL/GL_renderer.h"
#endif

#include "config/imgui_config.h"
#include "dashboard/dashboard.h"

#include "application.h"

// FORWARD DECLARATIONS ================================================================================================


namespace AT {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    // STATIC VARIABLES ================================================================================================

    application* application::s_instance = nullptr;
    ref<window> application::s_window;
    bool application::s_running;

    // FUNCTION IMPLEMENTATION =========================================================================================

    // CLASS IMPLEMENTATION ============================================================================================

    application::application(int argc, char* argv[])
        : m_fps_limiter(AT::util::fps_limiter()) {

        PROFILE_APPLICATION_FUNCTION();

        ASSERT(!s_instance, "", "Application already exists");
        LOG_INIT
        s_instance = this;

        // ----------- general subsystems -----------
    #if defined(PLATFORM_LINUX)
        util::init_qt();
    #endif
        s_window = std::make_shared<window>();
        s_window->show_titlebar(false);
        s_window->set_event_callback(BIND_FUNCTION(application::on_event));

        // ----------- general subsystems -----------
    #if defined(RENDER_API_OPENGL)
        m_renderer = create_ref<AT::render::open_GL::GL_renderer>(s_window);
    #elif defined(RENDER_API_VULKAN)
        m_renderer = create_ref<AT::render::vulkan::VK_renderer>(s_window);
    #endif

        // ----------- user defined system -----------
        m_imgui_config = create_ref<UI::imgui_config>();
        m_dashboard = create_ref<dashboard>();
        m_crash_subscription = AT::crash_handler::subscribe([this]() { m_dashboard->on_crash(); });
    }


    application::~application() {

        PROFILE_APPLICATION_FUNCTION();

        m_dashboard.reset();
        AT::crash_handler::unsubscribe(m_crash_subscription);
        m_imgui_config.reset();

        m_renderer->resource_free(); // need to call free manually because some destructors need the applications access to the renderer (eg: image)
        m_renderer.reset();
        s_window.reset();

        #if defined(PLATFORM_LINUX)
            util::shutdown_qt();
        #endif
        LOG_SHUTDOWN
    }

    // CLASS PUBLIC ====================================================================================================

    // CLASS PROTECTED =================================================================================================

    void application::run() {

        PROFILE_APPLICATION_FUNCTION();

        // ---------------------------------------- finished setup ----------------------------------------
        bool long_startup_process = false;
		AT::serializer::yaml(config::get_filepath_from_configtype(util::get_executable_path(), config::file::app_settings), "general_settings", AT::serializer::option::load_from_file)
			.entry(KEY_VALUE(long_startup_process));

        LOG(Info, "long_startup_process [" << util::to_string(long_startup_process) << "]")

        s_running = true;
        m_renderer->set_state(system_state::active);
        s_window->show_window(true);
        if (long_startup_process) {

            PROFILE_APPLICATION_SCOPE("long startup process(different thread)");

            std::atomic<bool> running_init = true;
            std::future<bool> init_future = std::async(std::launch::async, [this, &running_init]() {

                AT::logger::register_label_for_thread("client_init");
                bool result = m_dashboard->init();
                running_init = false;
                AT::logger::unregister_label_for_thread();
                return result;
            });

            while (running_init) {

                s_window->poll_events();				    // update internal state
                if (!s_running) {                           // Handle early termination
                    init_future.wait();                     // Ensure thread finishes before shutdown
                    break;                                  // Skip main loop entirely
                }

                m_renderer->draw_startup_UI(m_delta_time);
                m_fps_limiter.limit();
            }

        } else {

            PROFILE_APPLICATION_SCOPE("startup process(main thread)");

            m_dashboard->init();
            s_window->poll_events();
        }

        // ---------------------------------------- main loop ----------------------------------------
        while (s_running) {

            PROFILE_APPLICATION_SCOPE("main loop");
            s_window->poll_events();                        // update internal state
            m_dashboard->update(m_delta_time);
            m_renderer->draw_frame(m_delta_time);
            m_fps_limiter.limit();
        }

        {
            PROFILE_APPLICATION_SCOPE("Exiting main loop");
            LOG(Trace, "Exiting main run loop")
            m_dashboard->shutdown();
        }
    }

    // CLASS PRIVATE ===================================================================================================

    void application::on_event(event& event) {

        // application events
        event_dispatcher dispatcher(event);
        dispatcher.dispatch<window_close_event>(BIND_FUNCTION(application::on_window_close));
        dispatcher.dispatch<window_resize_event>(BIND_FUNCTION(application::on_window_resize));
        dispatcher.dispatch<window_refresh_event>(BIND_FUNCTION(application::on_window_refresh));
        dispatcher.dispatch<window_focus_event>(BIND_FUNCTION(application::on_window_focus));

        // none application events
        m_dashboard->on_event(event);
    }


    bool application::on_window_close(window_close_event& event) {

        s_running = false;
        return true;
    }


    bool application::on_window_resize(window_resize_event& event) {

        m_renderer->set_window_size(event.get_width(), event.get_height());
        return true;
    }


    bool application::on_window_refresh(window_refresh_event& event) {

        m_fps_limiter.limit();
        return true;
    }


    bool application::on_window_focus(window_focus_event& event) {

        m_fps_limiter.set_focus_state(event.get_focus());
        return true;
    }

}
