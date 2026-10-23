// in last lesson we made a proper working pipeline. 

// we bypassed HTTP and wrote a raw TCP pipeline !!

// this lesson will be dedicated to 

// GRACEFUL TEARFOWN
// To release the port instantly and free up the kernel's 
// memory, you must explicitly close both file descriptors 
// before the program exits.

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

int main() {
    // opening an fd for the server 
    int fd = socket(AF_INET , SOCK_STREAM , 0) ; // ipV4 , TCP
    // then we open a client 
    struct sockaddr_in addr ;
    addr.sin_family = AF_INET ; // 0.0.0.0
    addr.sin_port = htons(8080) ;
    addr.sin_addr.s_addr = INADDR_ANY ; 
    // we bind the server to the client 
    int fdt = bind(fd , (struct sockaddr*)&addr , sizeof(addr));
    // now we will put a fd for the client to listen
    int clt = listen(fd , 10);
    // now we will client for the waiting queue
    struct sockaddr_in client ; 
    socklen_t len = sizeof(client) ;
    int cl_fd = accept(fd , (struct sockaddr*)&client , &len);
    // now we have to wait for the client to write and press enter 
    char buffer[1024] = {0};
    ssize_t bytes_read = recv(cl_fd , buffer , sizeof(buffer) , 0);
    std::cout << buffer << std::endl;
    const char *response = "GOT IT !! Hows you client" ; 
    send(cl_fd , response , strlen(response) , 0);
    // closing all the fd's that we opened 
    // close in reverse order 
    close(cl_fd) ; 
    close(fd) ;
    return 0 ;
}
// ENOTCONN - Transport endpoint is not connected).