// To solve the C10K problem, we must let the macOS kernel 
// do the heavy lifting. Instead of asking 10,000 times if 
// data is ready, we hand the kernel a list of file 
// descriptors and say: "Put my thread to sleep. 
// Wake me up only when one of these has data."

// today the topic is : The kqueue Event Monitor

// The kqueue Event Monitor
// The macOS subsystem for this is kqueue. 
// It is an event monitor that acts as a master file descriptor.

// We will create the monitor, 
// then use a macro called EV_SET to package an instruction:
// "Watch the server socket (fd) for incoming connections 
// (READ events)." We push this instruction to the kernel 
// using the kevent() system call.

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/event.h>
#include <iostream>
#include <string_view>
#include <unistd.h>
#include <cstring>

int main() {
    // server fd 
    int fd = socket(AF_INET , SOCK_STREAM , 0);
    // non blocking flag 
    int flags = fcntl(fd , F_GETFL , 0);
    flags = fcntl(fd , F_SETFL , flags | O_NONBLOCK);
    // client 
    struct sockaddr_in addr ;
    addr.sin_family = AF_INET ;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY ; 
    // binding them together 
    int bd = bind(fd , (struct sockaddr*)&addr , sizeof(addr));
    // making a waiting queue 
    int lst = listen(fd , 10);
    // inserting this class function that is kqueue
    // we configured the watch instruction 
    int kq = kqueue() ;
    struct kevent ev_Set ;
    EV_SET(&ev_Set , fd , EVFILT_READ , EV_ADD | EV_ENABLE , 0 , 0 , NULL);
    // we can check this by : 
    if(kevent(kq , &ev_Set , 1 , NULL , 0 , NULL) == -1) {
        std::cout << "Failed to register server socket." << std::endl;
    }
    while(true) {
        // making an addr for the client 
        struct sockaddr_in client ;
        socklen_t len = sizeof(client);
        int cl_fd = accept(fd , (struct sockaddr*)&client , &len);
        if(cl_fd == -1) continue;

        // making a buffer for whatever the user sends 
        char buffer[1024] = {0};
        ssize_t bytes_read = recv(cl_fd , buffer , sizeof(buffer) , 0);

        if(bytes_read > 0) {
            // now we will parse the data that came 
            std::string_view str(buffer);
            int i = str.find(" ");
            std::string_view method = str.substr(0 , i);
            int j = str.find(" " , i + 1);
            std::string_view route = str.substr(i + 1 , j - (i + 1));

            // part for the headers needs to be extracted in key : value pairs 
            int start = str.find("\r\n");
            start = start + 2 ;
            while(start <= bytes_read) {
                int end = str.find("\r\n" , start);
                std::string_view header = str.substr(start , end - start);
                if(header.length() == 0) {
                    std::cout << "Header FINISHED" << std::endl;
                    break;
                } 
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
            send(cl_fd, http_response, strlen(http_response), 0);
        }
        close(cl_fd);
    }
    close(fd);
    return 0 ;
}

// now even when we connected the kqueue , yet if we 
// run this code in terminal using "top" we will see

// and you will see that this is still running on the 
// polling architecture and not the event-driven 
// architecture.

