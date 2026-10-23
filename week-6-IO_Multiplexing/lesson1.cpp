// Week 6: I/O Multiplexing (The C10K Problem)
// If you add the while(true) loop to your current server, 
// it will work perfectly for you testing locally. 
// But if you deploy this to production, 
// a single malicious user can take your entire backend offline 
// with zero effort.

// Here is how they do it:
// They connect to your server. 
// Your while loop hits accept() and unblocks.
// Your code moves to recv(), waiting for their HTTP headers.
// The malicious user sends nothing.
// Because recv() is a blocking system call, 
// your entire C++ thread goes to sleep waiting for data 
// that will never arrive. If a second user tries to connect, 
// they are stuck in the kernel's backlog queue forever. 
// Your server is completely deadlocked.

// To build a backend that handles 10,000 concurrent 
// connections on a single thread (like Node.js, Redis, 
// or Nginx), we must rip out blocking I/O and talk 
// directly to the macOS event kernel.

// To fix the deadlock, we must tell the OS kernel: 
// "Do not ever put my thread to sleep. If I call recv() 
// and the client hasn't sent data yet, just return an error 
// immediately so I can go do other things."

// The fcntl() System Call
// POSIX provides File Control (fcntl) to dynamically change 
// how a file descriptor behaves while the program is running. 
// We will use it to flip the O_NONBLOCK hardware flag on your 
// server socket.

// Your Action Item: Disable Blocking
// Include <fcntl.h>. Right after you create your server socket 
// (but before you bind it), read its current flags, append the 
// non-blocking flag, and write the flags back to the OS.


#include <sys/socket.h>
#include <netinet/in.h>
#include <cstddef>
#include <string_view>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

int main() {
    // making an fd for the server (ipV4 + TCP) 
    int fd = socket(AF_INET , SOCK_STREAM , 0);
    int flags = fcntl(fd , F_GETFL , 0);
    fcntl(fd , F_SETFL , flags | O_NONBLOCK);
    // then we make a client 
    struct sockaddr_in addr ; 
    addr.sin_family = AF_INET ; 
    addr.sin_port = htons(8080) ;
    addr.sin_addr.s_addr = INADDR_ANY ; // 0.0.0.0
    // now we need to bind those two and that connection a fd 
    size_t skt = bind(fd , (struct sockaddr*)&addr , sizeof(addr));
    // making a waiting queue for the client 
    size_t lsn = listen(fd , 10);
    // making a client that could address every query that comes to this queue
    while(true) {
        struct sockaddr_in client ;
        socklen_t len = sizeof(client);
        int cl_fd = accept(fd , (struct sockaddr*)&client , &len);
        if(cl_fd == -1) continue ;
        // all set now we can make a buffer where the input will be kept 
        char buffer[1024] = {0};
        // earlier the connection was so fast that it all was working 
        // in blink of an eye.
        // adding usleep(100000) , a 1 sec delay to demonstrate it.
        usleep(100000);
        ssize_t bytes_read = recv(cl_fd , buffer , sizeof(buffer) , 0);
        if(bytes_read > 0) {
            std::cout << buffer << std::endl;
            // now we have to process this with the parser 
            std::string_view str(buffer);
            size_t space = str.find(' ');
            std::string_view method = str.substr(0 , space);
            std::cout << "METHOD : " << method << std::endl;
            size_t sec_space = str.find(' ' , space + 1);
            std::string_view route = str.substr(space + 1 , sec_space - space);
            std::cout << "ROUTE : " << route << std::endl;
            // now we want to find the headers , they are surrounded by \r\n
            size_t start = sec_space + 2 ; 
            size_t end = str.find("\r\n" , start);
            while(end <= str.length()) {
                std::string_view header = str.substr(start , end - start);
                if(header.length() == 0) {
                    std::cout << "HEADER FINISHED" << std::endl;
                    break;
                } 
                size_t colon = header.find(":");
                if(colon != std::string_view::npos) {
                    std::string_view key = header.substr(0 , colon);
                    std::string_view value = header.substr(colon + 1);
                    if(value.length() > 0 && value[0] == ' ') {
                        value.remove_prefix(1);
                    }
                    std::cout << key << " : " << value << std::endl;
                }
                start = end + 2 ; // skips \r\n again !!! 
                end = str.find("\r\n" , start);
            }
            const char* http_response = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 13\r\n"
                "\r\n"
                "Hello Browser";
            send(cl_fd , http_response , strlen(http_response) , 0);
        }
        // closing the client's fd in reverse order    
        close(cl_fd);
    }
    close(fd);
    return 0; 
}


// To handle 10,000 users simultaneously, 
// you would have to keep their connections open 
// and store all 10,000 client file descriptors 
// in an array. Because the sockets are non-blocking 
// and you have no event monitor, your while(true) 
// loop must manually iterate over all 10,000 clients 
// on every single pass. You would call recv() 10,000 
// times per loop, millions of loops per second, just 
// asking the kernel if anyone sent a byte.

// The Physical Bottleneck:
// Syscall Overhead: Every recv() forces the CPU to switch 
// from User Space to Kernel Space. Doing this millions of 
// times per second wastes all CPU cycles on context switching.

// Latency: If client #9,999 sends data, they have to wait for 
// the CPU to pointlessly check the first 9,998 empty sockets 
// before their request gets processed.

// Thermal Throttling: Your CPU core will run at 100% 
// load continuously, generating massive heat while doing 
// absolutely no useful work.

// This O(N) busy-polling loop is exactly what caused 
// early web servers to crash during the late 1990s. 
// That hardware bottleneck was famously named the "C10K Problem".
