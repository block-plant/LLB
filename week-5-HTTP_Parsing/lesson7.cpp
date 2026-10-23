// in the last class we made a proper pipeline but that ends 
// as we get one header and we parse it.

// The Event Daemon (Keeping it Alive)
// Right now, your server handles exactly one browser request, 
// sends "Hello Browser", and then return 0 kills the program. 
// Real backend engines (like Nginx or Apache) run as 
// daemons—background processes that run forever.

// To make this a real server, you just wrap the accept() 
// phase in an infinite loop.


// 1. Setup: socket, bind, listen
    // ...

    // std::cout << "Server running on http://localhost:8080" << std::endl;

    // // 2. The Infinite Server Loop
    // while (true) {
    //     struct sockaddr_in client;
    //     socklen_t len = sizeof(client);
        
    //     // Thread blocks here until a browser connects
    //     size_t cl_fd = accept(fd, (struct sockaddr*)&client, &len);
        
    //     char buffer[1024] = {0};
    //     size_t bytes_read = recv(cl_fd, buffer, sizeof(buffer), 0);
        
    //     if (bytes_read > 0) {
    //         // ... [Your FIXED string_view parsing logic here] ...

    //         const char* http_response = 
    //             "HTTP/1.1 200 OK\r\n"
    //             "Content-Length: 13\r\n"
    //             "\r\n"
    //             "Hello Browser";
    //         send(cl_fd, http_response, strlen(http_response), 0);
    //     }
        
    //     // Close the client connection, but KEEP the server fd open
    //     close(cl_fd); 
    // }

    // // This code is now unreachable, which is what we want for a daemon.
    // close(fd);
    // return 0;


// just binding this all into a while(true) is just the task of the 
// class.
