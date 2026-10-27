// in last lesson we successfully solved the C10K problem 

// The Layer 4 TCP Proxy

// Right now, your engine is a destination. 
// Clients connect, you read their data, and 
// you send a hardcoded response.

// To turn this into a Layer 4 proxy, 
// your engine must become a bridge. 
// It will no longer read HTTP headers or 
// care if the payload is a web page, a database query, 
// or a video stream. It will blindly take bytes from 
// the client and shove them into a backend server, 
// and vice versa.

// To do this across 8 CPU cores safely, 
// you need three new architectural components:

// 1. The Backend Target
// You need a dummy server to forward traffic to. 
// Open a new terminal tab and start Python's 
// built-in web server on port 9000:

// python3 -m http.server 9000

// 2. The Thread-Safe Routing Table
// When kqueue wakes up a thread and hands it a socket with data, 
// the thread needs to know where to forward those bytes. 
// You will use a Hash Map to pair the Client File Descriptor 
// with the Backend File Descriptor. Because 8 cores will access 
// this map simultaneously, you must protect it with a Mutex to 
// prevent memory corruption.

// 3. Edge-Triggered Polling (EV_CLEAR)
// In Day 5, you used EV_ONESHOT because the client sent 
// one request, you sent one response, and the connection 
// closed. A proxy must stay alive for continuous, 
// bidirectional traffic. You will replace EV_ONESHOT 
// with EV_CLEAR (Edge-Triggered mode). 
// The kernel will only notify you when the state 
// changes from empty to having data, 
// solving the Thundering Herd race condition 
// without permanently disarming the socket.


// What is a Layer 4 Proxy?
// Layer 4 refers to the Transport layer of the OSI model (TCP/UDP). 
// A Layer 4 proxy does not read HTTP headers, parse JSON, or 
// understand SQL queries. It only sees a raw stream of binary data.

// In your previous Day 5 engine (which operated at Layer 7), 
// your code physically looked for the \r\n characters to 
// parse HTTP routes. A Layer 4 proxy is completely blind 
// to the payload. It acts as a high-speed pipe: it takes 
// bytes from a client socket and instantly shoves them 
// into a backend server socket, without ever looking at 
// what those bytes mean. Because it skips parsing, 
// it is incredibly fast.

// What is Our New Goal?
// The C10K problem was about capacity—proving your 
// engine could physically hold 10,000 open file 
// descriptors across multiple CPU cores without 
// deadlocking.

// Your new goal is distribution.

// Real-world systems are never a single backend server; 
// they are clusters of machines. Your goal now is to 
// build the exact infrastructure component that sits 
// in front of those clusters (like an AWS Network Load 
// Balancer or HAProxy).

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/event.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>

int main() {
    // 1. The Routing Table
    std::unordered_map<int, int> mp; 
    std::mutex map_lock;
    int fd = socket(AF_INET , SOCK_STREAM , 0);
    int opt = 1 ;
    setsockopt(fd , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt));
    int flags = fcntl(fd , F_GETFL , 0);
    flags = fcntl(fd , F_SETFL , flags | O_NONBLOCK);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET ;
    addr.sin_addr.s_addr = INADDR_ANY ; 
    addr.sin_port = htons(8080);
    int bnd = bind(fd , (struct sockaddr*)&addr , sizeof(addr));
    int lst = listen(fd , 10000);

    int kq = kqueue() ;
    struct kevent ev_Set ; 
    EV_SET(&ev_Set , fd , EVFILT_READ , EV_ADD | EV_ENABLE , 0 , 0 , NULL);
    kevent(kq , &ev_Set , 1 , NULL , 0 , NULL);

    auto worker = [&]() -> void {
        while(true) {
            struct kevent evList[32];
            int num = kevent(kq , NULL , 0 , evList , 32 , NULL);
            for(int i = 0 ; i < num ; i++) {
                int triggered_fd = evList[i].ident ;
                if(triggered_fd == fd) {
                    // drain the entire client room 
                    while(true) {
                        // this means create a client and put that on read alert
                        struct sockaddr_in client ;
                        socklen_t len = sizeof(client) ;
                        int cl_fd = accept(fd , (struct sockaddr*)&client , &len);
                        if(cl_fd == -1) break ;

                        // 2. Open a new socket to the Backend (Port 9000)
                        int bk_fd = socket(AF_INET , SOCK_STREAM , 0);
                        struct sockaddr_in addr_bk = {0};
                        addr_bk.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // this is 127.0.0.1
                        addr_bk.sin_family = AF_INET ;
                        addr_bk.sin_port = htons(9000);
                        // ACTUALLY CONNECT TO PYTHON
                        connect(bk_fd, (struct sockaddr*)&addr_bk, sizeof(addr_bk));
                        // Make both sockets non-blocking
                        int cl_flags = fcntl(cl_fd , F_GETFL , 0);
                        fcntl(cl_fd , F_SETFL , cl_flags | O_NONBLOCK);
                        int bk_flags = fcntl(bk_fd , F_GETFL , 0);
                        fcntl(bk_fd , F_SETFL , bk_flags | O_NONBLOCK);

                        // 3. Map them together safely
                        map_lock.lock();
                        mp[cl_fd] = bk_fd;
                        mp[bk_fd] = cl_fd; 
                        map_lock.unlock();
                        struct kevent newConn[2] ; 
                        // 4. Register both to kqueue using EV_CLEAR
                        EV_SET(&newConn[0] , cl_fd , EVFILT_READ , EV_ADD | EV_ENABLE | EV_CLEAR , 0 , 0 , NULL);
                        EV_SET(&newConn[1] , bk_fd , EVFILT_READ , EV_ADD | EV_ENABLE | EV_CLEAR , 0 , 0 , NULL);
                        kevent(kq , newConn , 2 , NULL , 0 , NULL);
                    }
                }
                else {
                    // 5. The Forwarding Logic
                    // Look up the destination FD for this triggered FD
                    map_lock.lock();
                    int target_fd = mp[triggered_fd];
                    map_lock.unlock();
                    // parsing the data of this client 
                    char buffer[4096] = {0};
                    ssize_t bytes_read = recv(triggered_fd , buffer , sizeof(buffer) , 0);

                    if(bytes_read > 0) {
                        send(target_fd , buffer , bytes_read , 0);
                    }   
                    else if(bytes_read == 0) {
                        close(triggered_fd);
                        close(target_fd);

                        map_lock.lock();
                        mp.erase(target_fd);
                        mp.erase(triggered_fd);
                        map_lock.unlock();
                    }
                }
            }
        }
    };
    unsigned int core_count = std::thread::hardware_concurrency() ; 
    std::cout << "Booting Engine on " << core_count << " CPU cores..." << std::endl;

    std::vector<std::thread> thread_pool ; 
    for(int i = 0 ; i < core_count ; i++) {
        thread_pool.emplace_back(worker);
    }
    for(auto &t : thread_pool) {
        t.join();
    }
    close(fd);
    return 0 ; 
}   
