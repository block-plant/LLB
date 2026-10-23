// this lesson is dedicated to socket call.

// it means Requesting a network File Descriptor (FD) from the OS kernel.

// In Unix-based systems (macOS, Linux), 
// the network layer operates on one absolute rule: 
// Everything is a file.

// When you want to talk to the internet, 
// you do not create a network object. 
// You ask the Operating System kernel to open a file. 
// The OS hands you back a simple integer called a File Descriptor (FD).

// By default, your terminal already has three files open: 
// 0 (Input), 1 (Output), and 2 (Error). 
// When you ask the OS for your first network connection, 
// it usually hands you FD 3.

// The socket() System Call
// To get this integer, 
// we use the raw POSIX API: 
// socket(domain, type, protocol).

// Domain (AF_INET): Tells the OS we want an IPv4 connection.

// Type (SOCK_STREAM): Tells the OS we want a reliable, 
// ordered connection. This forces the OS to use TCP.

// Protocol (0): Setting this to 0 tells the OS to use the 
// default protocol for the chosen type.

// If the OS allocates the internal memory successfully, 
// it returns a positive integer. If it fails, it returns -1.

#include <iostream>
#include <sys/socket.h> // The core POSIX network API

int main() {
    // creating an ipV4 TCP connection
    int fd = socket(AF_INET , SOCK_STREAM , 0);
    if(fd == -1) {
        std::cout << "Not Created" << std::endl;
    }
    else {
        std::cout << fd << std::endl;
    }
    // here we got 3 as the answer (output).
    return 0;
}