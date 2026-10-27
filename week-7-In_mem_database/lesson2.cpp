// today is the day 2 , today we will parse the string that the client gives

// today we run commands that we hardcoded they are yet not active !! 

#include <cstddef>
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
#include <string_view> 
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
        while (true) {
            struct kevent evList[32];
            int num = kevent(kq, NULL, 0, evList, 32, NULL);
            for (int i = 0; i < num; i++) {
                int triggered_fd = evList[i].ident;
                if (triggered_fd == fd) {
                    while (true) {
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

                    if(bytes > 0) {
                        // parsing will happen here
                        std::string_view str(buffer , bytes);
                        while(!str.empty() && (str.back() == '\n' || str.back() == '\r')) {
                            str.remove_suffix(1);
                        }
                        size_t space = str.find(' ');
                        std::string_view method = str.substr(0 , space);
                        if(method == "SET") {
                            if(space != std::string_view::npos) {
                                size_t second = str.find(' ' , space + 1);
                                if(second != std::string_view::npos) {
                                    std::string_view key = str.substr(space + 1, second - (space + 1));
                                    std::string_view value = str.substr(second + 1);
                                    send(triggered_fd , "+OK (SET triggered)\n" , 20 , 0);
                                } 
                                else {
                                    send(triggered_fd , "-ERROR invalid SET format\n" , 26 , 0);
                                }
                            }
                        }   
                        else if(method == "GET") {
                            if(space != std::string_view::npos) {
                                std::string_view key = str.substr(space + 1);
                                send(triggered_fd , "+OK (GET triggered)\n" , 20 , 0);
                            }
                        }
                        else if(method == "DEL") {
                            if(space != std::string_view::npos) {
                                std::string_view key = str.substr(space + 1);
                                send(triggered_fd , "+OK (DEL triggered)\n" , 20 , 0);
                            }
                        }
                        else {
                            send(triggered_fd , "-ERROR unknown command\n" , 23 , 0);
                        }
                    } 
                    else {
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
