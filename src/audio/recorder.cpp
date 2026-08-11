
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include "util/pch.h"
#include "audio/recorder.h"


// FORWARD DECLARATIONS ================================================================================================

namespace AT::audio {

    // CONSTANTS =======================================================================================================

    // MACROS ==========================================================================================================

    // TYPES ===========================================================================================================

    // STATIC VARIABLES ================================================================================================

    // FUNCTION IMPLEMENTATION =========================================================================================

    static void source_list_callback(pa_context* c, const pa_source_info* info, int eol, void* userdata) {

        auto* sources = static_cast<std::vector<source_info>*>(userdata);
        if (eol == 0 && info->monitor_of_sink != PA_INVALID_INDEX) {
            sources->push_back({info->index, info->name, info->description});
        }
    }


    static void context_state_callback(pa_context* c, void* userdata) {

        auto* mainloop = static_cast<pa_threaded_mainloop*>(userdata);
        pa_threaded_mainloop_signal(mainloop, 0);
    }


    static void write_wav_header(FILE* f, int sample_rate, int channels, int bits_per_sample, int data_size) {
        
        int byte_rate = sample_rate * channels * bits_per_sample / 8;
        int block_align = channels * bits_per_sample / 8;
        int chunk_size = 36 + data_size;

        fwrite("RIFF", 1, 4, f);
        fwrite(&chunk_size, 4, 1, f);
        fwrite("WAVE", 1, 4, f);
        fwrite("fmt ", 1, 4, f);
        int fmt_size = 16;
        fwrite(&fmt_size, 4, 1, f);
        short audio_format = 1; // PCM
        fwrite(&audio_format, 2, 1, f);
        short num_channels = static_cast<short>(channels);
        fwrite(&num_channels, 2, 1, f);
        fwrite(&sample_rate, 4, 1, f);
        fwrite(&byte_rate, 4, 1, f);
        fwrite(&block_align, 2, 1, f);
        short bits_per_sample_short = static_cast<short>(bits_per_sample);
        fwrite(&bits_per_sample_short, 2, 1, f);
        fwrite("data", 1, 4, f);
        fwrite(&data_size, 4, 1, f);
    }

    // CLASS IMPLEMENTATION ============================================================================================

    recorder::recorder()
        : m_recording(false), m_should_stop(false), m_bytes_recorded(0) {}


    recorder::~recorder() {

        stop_recording();
    }

    // CLASS PUBLIC ====================================================================================================

    std::vector<source_info> recorder::enumerate_devices() {

        std::vector<source_info> sources;

        pa_threaded_mainloop* mainloop = pa_threaded_mainloop_new();
        if (!mainloop)
            return sources;

        pa_mainloop_api* api = pa_threaded_mainloop_get_api(mainloop);
        pa_context* context = pa_context_new(api, "AudioRecorderEnum");
        if (!context) {
            pa_threaded_mainloop_free(mainloop);
            return sources;
        }

        pa_context_set_state_callback(context, context_state_callback, mainloop);

        if (pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
            pa_context_unref(context);
            pa_threaded_mainloop_free(mainloop);
            return sources;
        }

        pa_threaded_mainloop_lock(mainloop);
        pa_threaded_mainloop_start(mainloop);

        // Wait for context to become ready
        while (true) {
            pa_context_state_t state = pa_context_get_state(context);
            if (state == PA_CONTEXT_READY) break;
            if (!PA_CONTEXT_IS_GOOD(state)) {
                pa_threaded_mainloop_unlock(mainloop);
                pa_context_unref(context);
                pa_threaded_mainloop_stop(mainloop);
                pa_threaded_mainloop_free(mainloop);
                return sources;
            }
            pa_threaded_mainloop_wait(mainloop);
        }

        // Get source list
        bool op_done = false;
        pa_operation* op = pa_context_get_source_info_list(context, source_list_callback, &sources);
        if (op) {
            while (!op_done) {
                pa_threaded_mainloop_wait(mainloop);
                op_done = (pa_operation_get_state(op) == PA_OPERATION_DONE);
            }
            pa_operation_unref(op);
        }

        pa_threaded_mainloop_unlock(mainloop);

        // Cleanup
        pa_context_disconnect(context);
        pa_context_unref(context);
        pa_threaded_mainloop_stop(mainloop);
        pa_threaded_mainloop_free(mainloop);

        return sources;
    }


    bool recorder::start_recording(const std::string& device_name, const std::string& filename) {

        if (m_recording.load()) return false; // already recording

        m_should_stop = false;
        m_bytes_recorded = 0;
        {
            std::lock_guard<std::mutex> lock(m_time_mutex);
            m_start_time = std::chrono::steady_clock::now();
        }

        // Launch background thread
        m_thread = std::thread(&recorder::recording_thread, this, device_name, filename);
        m_recording = true;
        return true;
    }


    void recorder::stop_recording() {

        if (!m_recording.load()) return;
        m_should_stop = true;
        if (m_thread.joinable()) {
            m_thread.join();
        }
        m_recording = false;
    }


    std::chrono::seconds recorder::elapsed_time() const {

        std::lock_guard<std::mutex> lock(m_time_mutex);
        if (!m_recording.load()) return std::chrono::seconds(0);
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time);
    }

    // CLASS PROTECTED =================================================================================================

    // CLASS PRIVATE ===================================================================================================

    void recorder::recording_thread(const std::string& device_name, const std::string& filename) {

        // Open PulseAudio stream
        pa_sample_spec ss;
        ss.format = PA_SAMPLE_S16LE;
        ss.rate = 44100;
        ss.channels = 2;

        pa_channel_map map;
        pa_channel_map_init_stereo(&map);

        int error;
        pa_simple *s = pa_simple_new(nullptr, "recorder", PA_STREAM_RECORD,
                                    device_name.c_str(), "Recording", &ss, &map, nullptr, &error);
        if (!s) {
            std::cerr << "pa_simple_new failed: " << pa_strerror(error) << std::endl;
            m_recording = false;
            return;
        }

        FILE *f = fopen(filename.c_str(), "wb");
        if (!f) {
            std::cerr << "Cannot open output file: " << filename << std::endl;
            pa_simple_free(s);
            m_recording = false;
            return;
        }

        // Write placeholder header
        const int headerSize = 44;
        char header[headerSize] = {0};
        fwrite(header, 1, headerSize, f);
        fflush(f);

        const size_t BUFSIZE = 4096;
        u8 buffer[BUFSIZE];
        u64 totalBytes = 0;

        // Recording loop
        while (!m_should_stop.load()) {
            int ret = pa_simple_read(s, buffer, sizeof(buffer), &error);
            if (ret < 0) {
                std::cerr << "Read error: " << pa_strerror(error) << std::endl;
                break;
            }
            size_t written = fwrite(buffer, 1, sizeof(buffer), f);
            if (written != sizeof(buffer)) {
                std::cerr << "File write error" << std::endl;
                break;
            }
            totalBytes += written;
            m_bytes_recorded.store(totalBytes);
        }

        // Update header with actual data size
        fseek(f, 0, SEEK_SET);
        write_wav_header(f, ss.rate, ss.channels, 16, static_cast<int>(totalBytes));
        fclose(f);

        pa_simple_free(s);
        m_recording = false;
    }

}
