// now here we talk about the Client Extraction

// The accept() System Call
// We use the POSIX function: 
// accept(server_fd, client_address_pointer, address_size_pointer).

// The Thread Block (Sleeping)
// accept() is a blocking system call. 
// When your code reaches this line, your entire program halts. 
// The OS physically puts your C++ thread to sleep so it consumes 0% CPU.
// It will stay frozen there forever until a client actually connects. 
// The exact millisecond a client finishes the TCP handshake, 
// the OS wakes your thread up.

// The Second File Descriptor
// When accept() wakes up, 
// it does not use your original File Descriptor (FD 3). 
// Your original socket's only job is to stand at 
// the front door and listen.
// Instead, the OS kernel creates a brand new File Descriptor 
// (usually FD 4) just for this specific client. 
// You will use FD 4 to send and receive data, while FD 3 
// goes back to listening for the next person.

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream> 
int main() {
    // put on an fd at door
    int fd = socket(AF_INET , SOCK_STREAM , 0); // an ipv4 & TCP connections 
    if(fd == -1) return 1;
    // put on an connection with server 
    struct sockaddr_in server ;
    server.sin_family = AF_INET ;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY ;
    int s_fd = bind(fd , (struct sockaddr*)&server , sizeof(server));
    if(s_fd < 0) return 1; 
    // the waiting queue
    int skt = listen(fd , 10);
    if(skt < 0) return 1;
    // putting on a client to handle waiting queue
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    int client_fd = accept(fd , (struct sockaddr*)&client , &client_len);
    if(client_fd < 0) return 1;
    std::cout << "Connection established successfully !!" << std::endl;
    return 0 ; 
} 