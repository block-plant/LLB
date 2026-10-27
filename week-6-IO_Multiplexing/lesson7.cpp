// To upgrade your 1-to-1 proxy into a 1-to-N load balancer, 
// you will implement an atomic router that distributes traffic 
// across a cluster of backend servers.

// The Architecture
// When a client connects to port 8080, 
// your C++ engine will no longer hardcode the backend to port 9000. 
// Instead, it will read from a pool of available ports, 
// pick the next one in line, establish the physical connection, 
// and advance the counter. When the counter reaches the end of the pool, 
// it loops back to the start.

#include <cstddef>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <sys/event.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <mutex>

std::vector<int> backend_ports = {9000, 9001, 9002};
std::atomic<unsigned int> current_target{0};

int main() {
    std::unordered_map<int , int> mp;
    std::mutex lock; 
    int fd = socket(AF_INET , SOCK_STREAM , 0);
    int opt = 1 ;
    setsockopt(fd , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt));
    int flags = fcntl(fd , F_GETFL , 0);
    flags = fcntl(fd , F_SETFL , flags | O_NONBLOCK);
    struct sockaddr_in addr ;
    addr.sin_addr.s_addr = INADDR_ANY ; 
    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET ; 
    int bnd = bind(fd , (struct sockaddr*)&addr , sizeof(addr));
    int lst = listen(fd , 10000);

    int kq = kqueue();
    struct kevent evSet;
    EV_SET(&evSet , fd , EVFILT_READ , EV_ENABLE | EV_ADD , 0 , 0 , NULL);
    kevent(kq , &evSet , 1 , NULL , 0 , NULL);

    auto worker = [&]() -> void {
        while(true) {
            struct kevent evList[32];
            int num = kevent(kq , NULL , 0 , evList , 32 , NULL);
            for(int i = 0 ; i < num ; i++) {
                int triggered_fd = evList[i].ident ; 
                if(triggered_fd == fd) {
                    while(true) {
                        // add a cleint and put that the alert on listening 
                        struct sockaddr_in client ; 
                        socklen_t len = sizeof(client);
                        int cl_fd = accept(fd , (struct sockaddr*)&client , &len);
                        if(cl_fd == -1) {
                            break ;
                            // there is no child left in the queue
                        }
                        // fetch_add(1) atomically returns the current value and increments it in one CPU instruction
                        unsigned int target_index = current_target.fetch_add(1) % backend_ports.size();
                        int port = backend_ports[target_index];

                        // now making a backend port active 
                        int bk_fd = socket(AF_INET , SOCK_STREAM , 0);
                        struct sockaddr_in addrbk = {0}; 
                        addrbk.sin_family = AF_INET ; 
                        addrbk.sin_port = htons(port) ; 
                        addrbk.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                        
                        connect(bk_fd , (struct sockaddr*)&addrbk , sizeof(addrbk));
                        int cl_fg = fcntl(cl_fd , F_GETFL , 0);
                        cl_fg = fcntl(cl_fd , F_SETFL , cl_fg | O_NONBLOCK);
                        int bk_fg = fcntl(bk_fd , F_GETFL , 0);
                        bk_fg = fcntl(bk_fd , F_SETFL , bk_fg | O_NONBLOCK);

                        lock.lock();
                        mp[bk_fd] = cl_fd; 
                        mp[cl_fd] = bk_fd;
                        lock.unlock();

                        struct kevent newConn[2];
                        EV_SET(&newConn[0] , bk_fd , EVFILT_READ , EV_ADD | EV_ENABLE | EV_CLEAR , 0 , 0, NULL);
                        EV_SET(&newConn[1] , cl_fd , EVFILT_READ , EV_ADD | EV_ENABLE | EV_CLEAR , 0 , 0 , NULL);
                        kevent(kq , newConn , 2 , NULL , 0 , NULL);
                    }
                }
                else {
                    // just check the destination and send the data if available 
                    lock.lock();
                    int target_fd = mp[triggered_fd] ; 
                    lock.unlock();

                    char buffer[4096] = {0};
                    ssize_t bytes = recv(triggered_fd , buffer , sizeof(buffer) , 0);
                    
                    if(bytes > 0) {
                        send(target_fd , buffer , bytes , 0);
                    }
                    else if(bytes == 0) {
                        close(triggered_fd);
                        close(target_fd);

                        lock.lock();
                        mp.erase(target_fd);
                        mp.erase(triggered_fd);
                        lock.unlock();
                    }
                }
            }
        }
    };
    unsigned int core_count = std::thread::hardware_concurrency();
    std::cout << "Booting Engine on " << core_count << " CPU cores..." << std::endl;

    std::vector<std::thread> thread_pool ; 
    for(int i = 0 ; i < core_count ; i++) {
        thread_pool.emplace_back(worker);
    }
    for(auto&t : thread_pool) {
        t.join();
    }
    close(fd);
    return 0;
}


// Why Atomic Over Mutex

// A mutex works at the operating system level, 
// while std::atomic works at the physical silicon level.

// If you use a mutex for the counter, Thread 1 locks it, 
// and Threads 2-8 must completely halt, trigger a kernel context switch, 
// and go to sleep until the lock releases. In a C10K engine pushing 
// thousands of connections per second, this creates a massive traffic jam.

// An atomic variable uses a specialized hardware instruction on your M1 chip. 
// It updates the counter in a single CPU clock cycle without ever putting 
// the other threads to sleep. It provides the exact same memory safety as 
// a mutex but operates at nanosecond speeds.