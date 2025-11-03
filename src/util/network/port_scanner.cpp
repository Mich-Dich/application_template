#include "util/pch.h"

#if defined(PLATFORM_WINDOWS)
    #include <ws2tcpip.h>
    #include <winsock2.h>
    #include <mswsock.h>
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "mswsock.lib")
#elif defined(PLATFORM_LINUX)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

#include "port_scanner.h"

namespace AT::util {

    // Thread pool implementation with abort support
    port_scanner::thread_pool::thread_pool(size_t num_threads, std::atomic<bool>* abort_flag) : m_stop(false), m_abort_flag(abort_flag) {

        for (size_t i = 0; i < num_threads; ++i) {
            m_workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->m_queue_mutex);
                        this->m_condition.wait(lock, [this] {       // Check if we should stop (either pool stop or external abort)
                            return this->m_stop || (this->m_abort_flag && *this->m_abort_flag) || !this->m_tasks.empty();
                        });
                        
                        // Check for abort conditions
                        if ((this->m_stop && this->m_tasks.empty()) || 
                            (this->m_abort_flag && *this->m_abort_flag)) {
                            return;
                        }
                        
                        if (this->m_tasks.empty()) continue;
                        
                        task = std::move(this->m_tasks.front());
                        this->m_tasks.pop();
                    }
                    task();
                }
            });
        }
    }


    port_scanner::thread_pool::~thread_pool() { stop(); }


    void port_scanner::thread_pool::enqueue(std::function<void()> task) {

        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            // Don't enqueue if we're aborting
            if (m_stop || (m_abort_flag && *m_abort_flag))
                return;

            m_tasks.emplace(std::move(task));
        }
        m_condition.notify_one();
    }


    void port_scanner::thread_pool::wait_all() {

        while (true) {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            if (m_tasks.empty() || (m_abort_flag && *m_abort_flag))
                break;

            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }


    void port_scanner::thread_pool::stop() {

        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);
            m_stop = true;
            // Clear remaining tasks
            while (!m_tasks.empty())
                m_tasks.pop();
        }
        
        m_condition.notify_all();
        for (std::thread &worker : m_workers)
            if (worker.joinable())
                worker.join();
    }


    // Check if we should abort before even trying
    // (This function doesn't have access to abort_flag, but the caller can check)
    bool port_scanner::is_port_open(const std::string& ip, int port, int timeout_ms) {
        
        #if defined(PLATFORM_WINDOWS)

            SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == INVALID_SOCKET) return false;
            
            // Set socket to non-blocking
            u_long mode = 1;
            ioctlsocket(sock, FIONBIO, &mode);
            
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
            
            connect(sock, (sockaddr*)&addr, sizeof(addr));
            
            fd_set set, except_set;
            FD_ZERO(&set);
            FD_ZERO(&except_set);
            FD_SET(sock, &set);
            FD_SET(sock, &except_set);
            
            timeval timeout;
            timeout.tv_sec = timeout_ms / 1000;
            timeout.tv_usec = (timeout_ms % 1000) * 1000;
            
            int result = select(0, nullptr, &set, &except_set, &timeout);
            closesocket(sock);
            
            return result > 0 && FD_ISSET(sock, &set);

        #elif defined(PLATFORM_LINUX)

            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) return false;
            
            // Set socket to non-blocking
            int flags = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFL, flags | O_NONBLOCK);
            
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
            
            connect(sock, (sockaddr*)&addr, sizeof(addr));
            
            fd_set set, except_set;
            FD_ZERO(&set);
            FD_ZERO(&except_set);
            FD_SET(sock, &set);
            FD_SET(sock, &except_set);
            
            timeval timeout;
            timeout.tv_sec = timeout_ms / 1000;
            timeout.tv_usec = (timeout_ms % 1000) * 1000;
            
            int result = select(sock + 1, nullptr, &set, &except_set, &timeout);
            close(sock);
            
            return result > 0 && FD_ISSET(sock, &set);

        #endif
    }

    
    std::vector<int> port_scanner::scan_ports(const std::string& ip, int start_port, int end_port, std::function<void(float)> progress_callback,
        std::function<void(const std::string&)> status_callback, int thread_count, std::atomic<bool>* abort_flag) {

        if (status_callback) status_callback("Initializing...");
        
        // Check if we're already aborted
        if (abort_flag && *abort_flag) {
            if (status_callback) status_callback("Scan aborted");
            return {};
        }
        
        std::vector<int> open_ports;
        std::mutex ports_mutex;
        std::atomic<int> ports_scanned{0};
        const int total_ports = end_port - start_port + 1;
        
        // Create thread pool with abort flag
        thread_pool pool(thread_count, abort_flag);
        
        if (status_callback) status_callback("Starting parallel port scan...");
        
        // Common ports to scan first (priority)
        std::vector<int> common_ports = {21, 22, 23, 25, 53, 80, 110, 443, 993, 995, 
                                         5024, 5025, 5555, 8080, 8443};
        
        // Scan common ports first
        if (status_callback) status_callback("Scanning common ports...");
        for (int port : common_ports) {
            if (port < start_port || port > end_port)
                continue;

            // Check for abort before queuing each task
            if (abort_flag && *abort_flag) {
                if (status_callback) status_callback("Scan aborted");
                return open_ports;
            }
            
            pool.enqueue([&, port]() {
                // Check for abort at the start of the task
                if (abort_flag && *abort_flag) 
                    return;
                
                if (is_port_open(ip, port, 200)) {
                    std::lock_guard<std::mutex> lock(ports_mutex);
                    open_ports.push_back(port);
                }
                ports_scanned++;
                
                if (progress_callback) progress_callback(static_cast<float>(ports_scanned) / total_ports);
            });
        }
        
        // Scan remaining ports
        if (status_callback) status_callback("Scanning all ports...");
        for (int port = start_port; port <= end_port; port++) {
            
            // Skip if it's a common port (already queued)
            if (std::find(common_ports.begin(), common_ports.end(), port) != common_ports.end())
                continue;
            
            // Check for abort before queuing each task
            if (abort_flag && *abort_flag) {
                if (status_callback) status_callback("Scan aborted");
                break;
            }
            
            pool.enqueue([&, port]() {
                // Check for abort at the start of the task
                if (abort_flag && *abort_flag)
                    return;
                
                if (is_port_open(ip, port)) {
                    std::lock_guard<std::mutex> lock(ports_mutex);
                    open_ports.push_back(port);
                }
                ports_scanned++;
                
                if (progress_callback) progress_callback(static_cast<float>(ports_scanned) / total_ports);
            });
        }
        
        // Wait for all tasks to complete (or abort)
        if (status_callback) status_callback("Wait for thread pool to complete...");
        pool.wait_all();
        
        // Check if we aborted
        if (abort_flag && *abort_flag) {
            if (status_callback) status_callback("Scan aborted");
        } else {
            if (status_callback) status_callback("Sorting results...");
            std::sort(open_ports.begin(), open_ports.end());
        }
        
        return open_ports;
    }
}
