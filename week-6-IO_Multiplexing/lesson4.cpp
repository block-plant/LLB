// this class is dedicated towards removing usleep(5000)
// as all the users might not be sitting at a delay of 5mn on the wifi

// Managing Multiple Sockets (Removing the Hack)
// Right now, your usleep(5000) hack assumes every 
// client on Earth has a ping under 5 milliseconds. 
// If a user on a slow mobile network takes 200ms to 
// send their headers, your code waits 5ms, calls recv(), 
// gets -1, and drops their connection instantly. 
// If they are on a gigabit LAN, you are wasting 5ms 
// of pure CPU time doing nothing.

// To build a real C10K server, we must never guess. 
// We must hand the new client socket (cl_fd) to the 
// kqueue monitor and tell the OS: "Wake me up again 
// when this specific user actually sends their data."

// The Architectural Shift
// Up until now, your kqueue only watched one thing: 
// the server socket (fd). If it woke up, it meant a 
// new client was in the waiting room.
// Now, your kqueue will watch thousands of sockets 
// at the same time: 1 server socket + 9,999 client sockets.

// When kevent() wakes your thread up, how do you 
// know who woke you up?
// The macOS kernel tells you. 
// The evList array gets filled with kevent structs. 
// The .ident property of that struct contains the exact 
// file descriptor that triggered the event.

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/event.h>
#include <iostream>
#include <string_view>
#include <unistd.h>
#include <cstring>

int main() {
    int fd = socket(AF_INET , SOCK_STREAM , 0);
    int flags = fcntl(fd , F_GETFL , 0);
    flags = fcntl(fd , F_SETFL , flags | O_NONBLOCK);

    struct sockaddr_in addr ; 
    addr.sin_addr.s_addr = INADDR_ANY ; 
    addr.sin_port = htons(8080) ;
    addr.sin_family = AF_INET ;

    int bnd = bind(fd , (struct sockaddr*)&addr , sizeof(addr));
    int lst = listen(fd , 10);

    int kq = kqueue();
    struct kevent ev_Set ;
    EV_SET(&ev_Set , fd , EVFILT_READ , EV_ADD | EV_ENABLE , 0 , 0 , NULL);
    kevent(kq , &ev_Set , 1 , NULL , 0 , NULL);
    while(true) {
        struct kevent evList[32];
        int num = kevent(kq , NULL , 0 , evList , 32 , NULL);
        for(int i = 0 ; i < num ; i++) {
            int triggered_fd = evList[i].ident ; // this gives me the answer of who woke me up ?
            if(triggered_fd == fd) {
                // create a client 
                struct sockaddr_in client ;
                socklen_t len = sizeof(client);
                int cl_fd = accept(fd , (struct sockaddr*)&client , &len);
                if(cl_fd == -1) continue;

                // now i want to tell the kqueue to start looking for this client that when does it sends some data.
                struct kevent client_kd;
                EV_SET(&client_kd , cl_fd , EVFILT_READ , EV_ADD | EV_ENABLE , 0 , 0 , NULL);
                kevent(kq , &client_kd , 1 , NULL , 0 , NULL);
            }
            else {
                // here is the data arrived 
                char buffer[1024] = {0};
                ssize_t bytes_read = recv(triggered_fd , buffer , sizeof(buffer) , 0);
                if(bytes_read > 0) {
                    // now we have to parse it 
                    std::string_view window(buffer);
                    int space = window.find(" ");
                    std::string_view method = window.substr(0 , space);
                    int second_space = window.find(" " , space + 1) ;
                    std::string_view route = window.substr(space + 1 , (second_space - (space + 1)));
                    int start = window.find("\r\n") ; 
                    start = start + 2 ;
                    while(start <= bytes_read) {
                        int end = window.find("\r\n" , start);
                        std::string_view header = window.substr(start , end - start);
                        if(header.length() == 0) {
                            std::cout << "HEADER COMPLETED !" << std::endl;
                            break; 
                        }
                        // extracting key : value pair 
                        int colon = header.find(":");
                        std::string_view key = header.substr(0 , colon);
                        std::string_view val = header.substr(colon + 1);
                        if(val.length() > 0 && val[0] == ' ') {
                            val.remove_prefix(1);
                        }
                        std::cout << key << " : " << val << std::endl;
                        start = end + 2 ;
                    }
                    const char* http_response = 
                        "HTTP/1.1 200 OK\r\n"
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
    close(fd);
    return 0 ; 
}