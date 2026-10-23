// this class will involve memory addressing 
// (Endianness & sockaddr_in)

// Now that you have a raw network file, 
// you need to tell the OS which port it 
// should attach to (e.g., Port 8080). 
// But there is a massive hardware trap waiting for you: Endianness.

// Your Apple M1 chip uses Little-Endian architecture. 
// It stores integers backwards in physical RAM to 
// optimize CPU math operations.
// The internet standard is Big-Endian. 
// It expects integers forwards.

// If you tell your Mac to open Port 8080 
// (which is 1F90 in hexadecimal), 
// your CPU stores it backwards as 901F. 
// If you hand that directly to the network card, 
// it will blindly open Port 36895 instead.

// The Translation Layer: htons()
// To fix this, POSIX provides a hardware-level C function: 
// htons() (Host TO Network Short). It intercepts your 16-bit integer 
// (the port) and physically flips the bytes so the network card 
// reads it correctly.

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
int main() {
    struct sockaddr_in addr ; 
    addr.sin_family = AF_INET ; // this means ipV4
    addr.sin_port = htons(8080) ;
    addr.sin_addr.s_addr = INADDR_ANY ; // this is just 0.0.0.0
    std::cout << "Human Port : 8080" << std::endl;
    std::cout << "Network Port (Flipped): " << addr.sin_port << std::endl;
    // Human Port : 8080
    // Network Port (Flipped): 36895
    return 0;
}