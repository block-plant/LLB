// in this class we will be binding the two classes
// making the port ready and also making the socket ready 
// binding the socket and the port ready 

// We tell the Operating System: 
// "Take File Descriptor 3, 
// and strictly reserve Port 8080 for it. 
// If any other program tries to use Port 8080, block them."

// The bind() System Call
// We use the POSIX function: 
// bind(socket_fd, address_pointer, address_size).

// There is one major C-language quirk here you must understand: 
// Struct Casting.
// The bind() function was written in C, 
// decades before C++ classes or inheritance existed. 
// It expects a generic pointer called struct sockaddr*.
// But we are using an IPv4-specific struct called 
// struct sockaddr_in*.

// To make the compiler happy, we have to forcefully 
// cast our specific struct into the generic one using a 
// C-style cast: (struct sockaddr*)&addr. 
// This is how early systems engineers faked polymorphism in C.

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
int main() {
    // bring fd
    int fd = socket(AF_INET , SOCK_STREAM , 0);
    if(fd == -1) {
        std::cout << "failed" << std::endl;
        return 1; 
    }
    // bring port
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET;
    int skt = bind(fd , (struct sockaddr*)&addr , sizeof(addr));
    if(skt < 0) {
        std::cout << "Failed binding" << std::endl;
    }
    else {
        std::cout << "Working..." << std::endl;
    }
    return 0;
}