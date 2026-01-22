
#include "util/pch.h"
#include "timing_controller.h"

// FORWARD DECLARATIONS ================================================================================================

namespace AT::util
{

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    // STATIC VARIABLES ================================================================================================

    // FUNCTION IMPLEMENTATION =========================================================================================

    // CLASS IMPLEMENTATION ============================================================================================

    fps_limiter::fps_limiter() : m_stopwatch(&m_work_time, duration_precision::microseconds) { }

    // CLASS PUBLIC ====================================================================================================

    void fps_limiter::set_focused_fps(const u32 focused_fps) {

        m_focused_fps = focused_fps;
        m_target_duration = static_cast<f32>(1.0 / focused_fps);

        // Update current limit if focused
        if (m_is_focused) {
            set_focused_fps(focused_fps);
        }
    }


    void fps_limiter::set_focused_fps(const u32 focused_fps, const u32 unfocused_fps) {

        m_focused_fps = focused_fps;
        m_unfocused_fps = unfocused_fps;
        set_focus_state(m_is_focused);
    }


    void fps_limiter::set_focus_state(const bool focused) {

        m_is_focused = focused;
        set_focused_fps(focused ? m_focused_fps : m_unfocused_fps);
    }


    void fps_limiter::limit() {

        PROFILE_FUNCTION();

        m_stopwatch.stop();

        // Sleep if frame completed too quickly
        if ((m_work_time / 1000) < m_target_duration) {
            m_sleep_time = m_target_duration - (m_work_time / 1000);
            high_precision_sleep(m_sleep_time);
        } else {
            m_sleep_time = 0;
        }

        m_stopwatch.restart();

        // Update timing statistics
        m_delta_time = m_work_time + (m_sleep_time / 1000);
        m_absolute_time += m_work_time;
        m_current_fps = static_cast<u32>(1.0 / (m_work_time + (m_sleep_time * 0.001)) + 0.5);
    }

    // CLASS PROTECTED =================================================================================================

    // CLASS PRIVATE ===================================================================================================

}
