#pragma once

#include "util/util.h"


// FORWARD DECLARATIONS ================================================================================================

namespace AT::util {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    // STATIC VARIABLES ================================================================================================

    // FUNCTION DECLARATION ============================================================================================

    // TEMPLATE DECLARATION ============================================================================================

    // CLASS DECLARATION ===============================================================================================

    class fps_limiter {
    public:

        fps_limiter();
        ~fps_limiter() = default;

        // Getters
        DEFAULT_GETTER(u32, current_fps)
        DEFAULT_GETTER(f32, delta_time)
        DEFAULT_GETTER(f32, absolute_time)
        DEFAULT_GETTER(f32, work_time)
        DEFAULT_GETTER(f32, sleep_time)
        DEFAULT_GETTER(u32, focused_fps)
        DEFAULT_GETTER(u32, unfocused_fps)

        // Sets the target FPS and calculates target duration
        void set_focused_fps(const u32 focused_fps);

        // Sets separate FPS limits for focused and unfocused states
        void set_focused_fps(const u32 focused_fps,const u32 unfocused_fps);

        // Updates the current FPS limit based on focus state
        void set_focus_state(const bool focused);

        // Main FPS limiting function - call at the end of each frame
        void limit();

    private:

        u32             m_focused_fps = 60;
        u32             m_unfocused_fps = 30;
        u32             m_current_fps = 0;
        f32             m_delta_time = 0.f;
        f32             m_absolute_time = 0.f;
        f32             m_work_time = 0.f;
        f32             m_sleep_time = 0.f;
        f32             m_target_duration = 1.0f / 60.0f;
        bool            m_is_focused = true;
        stopwatch       m_stopwatch;
    };

}
