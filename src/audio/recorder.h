
#pragma once



// FORWARD DECLARATIONS ================================================================================================

namespace AT::audio {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    struct source_info {
        u32                 index;
        std::string         name;
        std::string         description;
    };

    // STATIC VARIABLES ================================================================================================

    // FUNCTION DECLARATION ============================================================================================

    // TEMPLATE DECLARATION ============================================================================================

    // CLASS DECLARATION ===============================================================================================
    
    class recorder {
    public:

        recorder();
        ~recorder();

        // Enumerate available monitor sources (call once at startup or when refreshing)
        static std::vector<source_info> enumerate_devices();

        // Control recording
        bool start_recording(const std::string& device_name, const std::string& filename);
        void stop_recording();

        // Status queries (thread-safe)
        bool is_recording() const { return m_recording.load(); }
        std::chrono::seconds elapsed_time() const;   // returns seconds since start
        u64 bytes_recorded() const { return m_bytes_recorded.load(); }

    private:

        void recording_thread(const std::string& device_name, const std::string& filename);

        std::atomic<bool>                       m_recording;
        std::atomic<bool>                       m_should_stop;
        std::atomic<u64>                        m_bytes_recorded;
        std::chrono::steady_clock::time_point   m_start_time;
        mutable std::mutex                      m_time_mutex;             // protects m_startTime
        std::thread                             m_thread;
    };

}
