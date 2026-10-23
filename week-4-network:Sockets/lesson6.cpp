// now we will be doing TCP Data Transfer 

// Because of the Unix rule that 
// "everything is a file," talking to the network 
// is exactly the same as reading and writing to a 
// text file. You just read bytes from the file 
// descriptor, and you write bytes into it.

// The recv() System Call
// POSIX function: recv(client_fd, buffer_pointer, max_bytes, flags)
// Just like accept(), recv() is a blocking call. Once the connection 
// is established, your thread will go back to sleep until the client 
// actually pushes data across the wire. When the data arrives,
// the OS dumps it directly into the raw memory buffer you provide.

#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>

int main() {
    // creating the server fd 
    int fd = socket(AF_INET , SOCK_STREAM , 0);
    // creating the port on my machine for it 
    struct sockaddr_in addr ;
    addr.sin_family = AF_INET ; 
    addr.sin_port = htons(8080) ;
    addr.sin_addr.s_addr = INADDR_ANY ; // 0.0.0.0
    // binding the port to fd 
    int skt = bind(fd , (struct sockaddr*)&addr , sizeof(addr));
    // how we want to have an fd so that client could listen on
    int lfd = listen(fd , 10) ; // listening from fd 
    // creating a client to handle the waiting queue
    struct sockaddr_in client ;
    socklen_t len = sizeof(client);
    int cl_fd = accept(fd , (struct sockaddr*)&client , &len);

    // all set up 
    // now we can send data and recieve it 

    char buffer[1024] = {0};
    ssize_t read = recv(cl_fd , buffer , sizeof(buffer) , 0);
    if(read < 0) return -1;
    std::cout << buffer << std::endl;
    const char* response = "Got the message !!" ; 
    send(cl_fd , response , strlen(response) , 0);
}

// teh code that will be running here will become the reciever 
// for the sender part : open any other terminal and write 
// nc localhost 8080
// and then type anything and press enter , 
// the response that is sent from here will be shown there and 
// the msg sent from there will be shown here.