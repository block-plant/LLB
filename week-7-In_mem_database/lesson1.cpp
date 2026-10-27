// The Architecture Shift: What You Are Building

// You are building an in-memory Key-Value database—a custom
// clone of Redis.

// Over what? You are keeping the exact C10K event-loop engine you
// built in Week 6. The socket(), bind(), listen(), kqueue() mechanics,
// and the 8-core thread pool remain untouched. That is your I/O Engine.
// It is the chassis that handles 10,000 concurrent network connections
// at zero cost.

// What changes is the Application Logic. You are ripping out the Layer 4
// Proxy router (the part that dials Python on port 9000).
// Instead of acting as a bridge to another server, your C++ engine is
// now the final destination. When a client sends bytes, you will parse
// them as database commands (SET, GET) and store the data directly in
// your C++ engine's RAM.
// above is the complete week's task mentioned

// Day 1: The Global Store & Readers-Writer Lock

// To transform the engine into a database, you must replace the
// proxy's file-descriptor map with a global data map.

// Because 8 CPU cores will access this data simultaneously,
// a standard std::mutex is too slow. If Core 1 is reading the username,
// Cores 2 through 8 shouldn't have to wait—reads don't corrupt memory.
// You will upgrade to std::shared_mutex (a Readers-Writer lock).
// It allows infinite simultaneous reads (GET), but instantly locks
// out everyone else when a core needs to write (SET).

#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <shared_mutex>
#include <string>
#include <sys/event.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string, std::string> kv_store;
// // commenting the below line just to remove the red mark from the file and folder
// std::shared_mutex store_lock; // this is an error because its not there in c++14 , but there in c++17 and further

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    int flags = fcntl(fd, F_GETFL, 0);
    flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET;

    int bnd = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    int lst = listen(fd, 10000);

    int kq = kqueue();
    struct kevent evSet;
    EV_SET(&evSet, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    kevent(kq, &evSet, 1, NULL, 0, NULL);

    auto worker = [&]() -> void {
        while(true) {
        struct kevent evList[32];
        int num = kevent(kq, NULL, 0, evList, 32, NULL);
        for(int i = 0; i < num; i++) {
            int triggered_fd = evList[i].ident;
            if(triggered_fd == fd) {
                while(true) {
                    struct sockaddr_in client;
                    socklen_t len = sizeof(client);
                    int cl_fd = accept(triggered_fd, (struct sockaddr *)&client, &len);
                    if (cl_fd == -1)
                    break;

                    int cl_fg = fcntl(cl_fd, F_GETFL, 0);
                    cl_fg = fcntl(cl_fd, F_SETFL, cl_fg | O_NONBLOCK);

                    struct kevent newConn;
                    EV_SET(&newConn, cl_fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR,
                        0, 0, NULL);
                    kevent(kq, &newConn, 1, NULL, 0, NULL);
                }
            } 
            else {
                char buffer[4096] = {0};
                ssize_t bytes = recv(triggered_fd, buffer, sizeof(buffer), 0);

                if (bytes > 0) {
                    // just telling OK for now will code further in later
                    send(triggered_fd, "+OK\n", 4, 0);
                } else {
                    close(triggered_fd);
                }
            }
        }
        }
    };
    unsigned int core_count = std::thread::hardware_concurrency();
    std::cout << "Booting on : " << core_count << " cores...." << std::endl;
    std::vector<std::thread> thread_pool;
    for(int i = 0; i < core_count; i++) {
        thread_pool.emplace_back(worker);
    }
    for(auto &t : thread_pool) {
        t.join();
    }
    close(fd);
    return 0;
}
