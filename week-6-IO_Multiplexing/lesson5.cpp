// The Shared Monitor Architecture
// You do not create a separate kqueue for each thread. 
// You create one master kqueue in your main() function, 
// and pass that single monitor to every thread.

// When 4 different CPU cores call kevent() on the exact 
// same monitor at the exact same time, the macOS kernel 
// acts as the traffic cop. If 10 clients connect simultaneously, 
// the kernel safely distributes those events across the sleeping 
// threads. It guarantees that two threads will never be handed 
// the exact same client socket.

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/event.h>
#include <iostream>
#include <string_view>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>

int main() {
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

    auto worker = [kq , fd]() -> void {
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

                        // FIX 1: Make the new client connection non-blocking
                        int cl_flags = fcntl(cl_fd , F_GETFL , 0);
                        fcntl(cl_fd , F_SETFL , cl_flags | O_NONBLOCK);
                        struct kevent client_kd ; 
                        // FIX 2: Add EV_ONESHOT to prevent the THUNDERING HERD RACE CONDITION
                        EV_SET(&client_kd , cl_fd , EVFILT_READ , EV_ADD | EV_ENABLE | EV_ONESHOT , 0 , 0 , NULL);
                        kevent(kq , &client_kd , 1 , NULL , 0 , NULL);
                    }
                }
                else {
                    // parsing the data of this client 
                    char buffer[1024] = {0};
                    ssize_t bytes_read = recv(triggered_fd , buffer , sizeof(buffer) , 0);

                    if(bytes_read > 0) {
                        std::string_view str(buffer);
                        int space = str.find(" ");
                        std::string_view method = str.substr(0 , space);
                        // std::cout << "METHOD" << " : " << method << std::endl ; 
                        int secondSpace = str.find(" " , space + 1);
                        std::string_view route = str.substr(space + 1 , secondSpace - (space + 1));
                        // std::cout << "ROUTE" << " : " << route << std::endl ; 
                        int start = str.find("\r\n");
                        start = start + 2 ; 
                        while(start <= bytes_read) {
                            int end = str.find("\r\n" , start);
                            std::string_view header = str.substr(start , end - start);
                            if(header.length() == 0) {
                                // std::cout << "HEADER FINISHED !" << std::endl;
                                break;
                            }
                            // key : val
                            int colon = header.find(":");
                            std::string_view key = header.substr(0 , colon) ;
                            std::string_view val = header.substr(colon + 1) ;
                            if(val.length() > 0 && val[0] == ' ') {
                                val.remove_prefix(1);
                            }
                            // std::cout << key << " : " << val << std::endl ; 
                            start = end + 2 ; 
                        }    
                        const char* http_response = 
                            "HTTP/1.1 200 OK\r\n"
                            "Connection: close\r\n"
                            "Content-Length: 13\r\n"
                            "\r\n"
                            "Hello Browser";
                        // Remember to send to triggered_fd, not cl_fd!
                        send(triggered_fd, http_response, strlen(http_response), 0);
                    }   
                    close(triggered_fd);
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




// Welcome to the hardest part of systems engineering. 
// You just encountered the Thundering Herd race condition. 
// This is the exact reason Nginx and Redis are so heavily optimized.

// Here is the millisecond timeline of why the browser 
// worked but curl crashed:

// curl connects and sends 75 bytes.
// Because your kqueue is Level-Triggered (the default), 
// it screams: "This socket has data!"

// Thread 1 wakes up, grabs the event, calls recv(), 
// gets the 75 bytes, and starts your string parsing loop.

// Because Thread 1 is busy parsing strings, 
// the kernel hasn't registered that the data is gone yet. 
// So kqueue wakes up Thread 2 and hands it the exact same event.

// Thread 2 calls recv(). The buffer is now empty. 
// recv() returns -1.

// Thread 2 falls through your if(bytes_read > 0) block and 
// instantly executes close(triggered_fd).

// Thread 1 finally finishes parsing the string and calls send(). 
// But Thread 2 already destroyed the socket. 
// The kernel drops the response, and curl prints: 
// Empty reply from server.

// The Architecture Fix
// To fix this, you must explicitly command the 
// kernel's scheduling logic.

// 1. Make the client socket Non-Blocking
// Right now, if a client connects but sends no data, 
// your thread will block on recv() forever, effectively 
// killing one of your CPU cores.

// 2. Use EV_ONESHOT
// You must tell kqueue to disarm the trigger the nanosecond
// it wakes up a single thread. This prevents Thread 2 from 
// ever seeing the event.