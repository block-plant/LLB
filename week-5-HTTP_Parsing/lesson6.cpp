// here we are fusing the TCP pipeline and the parser 

// TCP + PARSER FUSION 

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstddef>
#include <string_view>
#include <iostream>
#include <unistd.h>
#include <cstring>

int main() {
    // making an fd for the server (ipV4 + TCP) 
    size_t fd = socket(AF_INET , SOCK_STREAM , 0);
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
    struct sockaddr_in client ;
    socklen_t len = sizeof(client);
    size_t cl_fd = accept(fd , (struct sockaddr*)&client , &len);

    // all set now we can make a buffer where the input will be kept 
    char buffer[1024] = {0};
    size_t bytes_read = recv(cl_fd , buffer , sizeof(buffer) , 0);
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
    // closing the fd's in reverse order 
    close(cl_fd);
    close(fd);
    return 0; 
}