
#include "util/pch.h"

#include "util/math/math.h"

#include "stopwatch.h"

namespace AT::util {

    f32 stopwatch::stop() {

        std::chrono::system_clock::time_point end_point = std::chrono::system_clock::now();
        switch (m_precision) {
            case duration_precision::microseconds: return *m_result_pointer = std::chrono::duration_cast<std::chrono::nanoseconds>(end_point - m_start_point).count() / 1000.f;
            case duration_precision::seconds:      return *m_result_pointer = std::chrono::duration_cast<std::chrono::milliseconds>(end_point - m_start_point).count() / 1000.f;
            default:
            case duration_precision::milliseconds: return *m_result_pointer = std::chrono::duration_cast<std::chrono::microseconds>(end_point - m_start_point).count() / 1000.f;
        }
    }

    void stopwatch::restart() {

        _start();
    }


    void stopwatch::_start() { m_start_point = std::chrono::system_clock::now(); }

}
