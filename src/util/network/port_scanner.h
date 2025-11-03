#pragma once

namespace AT::util {

    class port_scanner {
    public:

        static std::vector<int> scan_ports(const std::string& ip, int start_port, int end_port, std::function<void(float)> progress_callback = nullptr, 
            std::function<void(const std::string&)> status_callback = nullptr, int thread_count = 500, std::atomic<bool>* abort_flag = nullptr);
        
        static bool is_port_open(const std::string& ip, int port, int timeout_ms = 500);

    private:
        
        class thread_pool {
        public:

            thread_pool(size_t num_threads, std::atomic<bool>* abort_flag = nullptr);
            ~thread_pool();

            void enqueue(std::function<void()> task);
            void wait_all();
            void stop();  // Add stop method
            
        private:
            std::vector<std::thread>            m_workers;
            std::queue<std::function<void()>>   m_tasks;
            std::mutex                          m_queue_mutex;
            std::condition_variable             m_condition;
            std::atomic<bool>                   m_stop;
            std::atomic<bool>*                  m_abort_flag;  // External abort flag
        };
    };
}