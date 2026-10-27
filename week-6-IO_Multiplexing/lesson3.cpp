// Week 6: The Event-Driven Architecture (0% CPU)
// We are abandoning the busy-polling loop that causes the C10K crash.
// Instead of manually asking the OS "is data ready?" millions of times per second,
// we hand the kernel a pager (kqueue). The kernel physically suspends our thread,
// using 0% CPU, and wakes us the exact nanosecond a hardware packet arrives.

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/event.h>
#include <iostream>
#include <string_view>
#include <unistd.h>
#include <cstring>

int main() {
    // 1. HARDWARE PIPELINE
    // Ask the OS for a raw IPv4 TCP endpoint.
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    // Strip away the socket's default blocking behavior.
    // If we call recv() and it's empty, it will fail instantly 
    // instead of freezing the thread.
    int flags = fcntl(fd, F_GETFL, 0);
    flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    
    // Claim port 8080 on this machine.
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY; 
    
    // Bind the socket to the port and create a waiting room 
    // (backlog of 10).
    int bd = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    int lst = listen(fd, 10);


    // 2. THE EVENT MONITOR (kqueue)
    // Ask the macOS kernel to create a master event monitor.
    int kq = kqueue();
    // Package an instruction for the kernel:
    // "Watch my server socket (fd). 
    // If a new client knocks (READ), track it."
    struct kevent ev_Set;
    EV_SET(&ev_Set, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    
    // Push the instruction to the kernel.
    if(kevent(kq, &ev_Set, 1, NULL, 0, NULL) == -1) {
        std::cout << "Failed to register server socket." << std::endl;
    }

    // 3. THE EVENT LOOP (0% CPU)
    while(true) {
        // Create an empty bucket for the kernel to drop 
        // triggered events into.
        struct kevent evList[32];
        // THE SLEEP CALL: The thread physically stops executing here. 
        // CPU usage drops to 0.0%. It only wakes up when the kqueue 
        // detects a hardware event.
        int num = kevent(kq, NULL, 0, evList, 32, NULL);
        std::cout << "\nKernel woke us up! Events pending: " << num << std::endl;
        
        // Bring the new client into our process.
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int cl_fd = accept(fd, (struct sockaddr*)&client, &len);
        
        // If the OS woke us up but the client dropped the connection, 
        // skip to the next loop.
        if(cl_fd == -1) continue;
        
        // THE HACK: cl_fd inherited the non-blocking flag from the server socket.
        // We must sleep for 5ms to give the HTTP text time to physically travel 
        // over the Wi-Fi.
        usleep(5000);
        
        // Read the raw bytes from the network card into our stack memory.
        char buffer[1024] = {0};
        ssize_t bytes_read = recv(cl_fd, buffer, sizeof(buffer), 0);

        if(bytes_read > 0) {
            // 4. ZERO-ALLOCATION PARSING
            
            // Cast the raw byte array into a string_view ptr. 
            // No heap allocation occurs.
            std::string_view str(buffer);
            
            // Slide the window to extract the Method and Route.
            int i = str.find(" ");
            std::string_view method = str.substr(0, i);
            int j = str.find(" ", i + 1);
            std::string_view route = str.substr(i + 1, j - (i + 1));

            // Move the pointer past the first line to start reading headers.
            int start = str.find("\r\n");
            start = start + 2;
            
            // Loop line by line until we hit the double blank line (\r\n\r\n).
            while(start <= bytes_read) {
                int end = str.find("\r\n", start);
                std::string_view header = str.substr(start, end - start);
                
                // Length 0 means we found the blank line separating headers from the body.
                if(header.length() == 0) {
                    std::cout << "Header FINISHED" << std::endl;
                    break;
                } 
                
                // Split the header into Key and Value using the colon delimiter.
                int colon = header.find(":");
                std::string_view key = header.substr(0, colon);
                std::string_view val = header.substr(colon + 1);
                
                // Strip the leading space off the value in O(1) time by advancing the pointer.
                if(val.length() > 0 && val[0] == ' ') {
                    val.remove_prefix(1);
                }
                std::cout << key << " : " << val << std::endl;
                
                // Advance the global pointer to the next line.
                start = end + 2; 
            }
            
            // 5. THE HTTP RESPONSE
            // The browser will hang forever unless we speak strictly 
            // formatted HTTP back to it.
            const char* http_response = 
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 13\r\n"
                    "\r\n"
                    "Hello Browser";
            send(cl_fd, http_response, strlen(http_response), 0);
        }
        // Hang up the phone.
        close(cl_fd);
    }
    // Unreachable daemon teardown.
    close(fd);
    return 0;
}