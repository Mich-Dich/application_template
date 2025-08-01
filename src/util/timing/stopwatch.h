#pragma once

namespace AT::util {

    // @brief This is a lightweit stopwatch that is automaticlly started when creating an instance.
    //        It can either store the elapsed time in a provided float pointer when the stopwatch is stopped/destroyed, 
    //        or it can allow retrieval of the elapsed time by manually calling [stop()] method.
    //        The time is measured in milliseconds.
    class stopwatch {
    public:

        stopwatch(f32* result_pointer, duration_precision presition = duration_precision::milliseconds)
            : m_result_pointer(result_pointer), m_presition(presition) { _start(); }

        ~stopwatch() { stop(); }

        DEFAULT_GETTER_C(f32, result);

        // @brief Stops the stopwatch and calculates the elapsed time.
        //        If a result pointer was provided, it will be updated with the elapsed time.
        // @return The elapsed time in milliseconds since the stopwatch was started.
        f32 stop();

        // @breif Restarts the stopped stopwatch
        void restart();

    private:

        void _start();

        f32*                                        m_result_pointer = &m_result;
        duration_precision                          m_presition;
        std::chrono::system_clock::time_point       m_start_point{};
        f32                                         m_result = 0.f;
    };

}
