// now we have the binding 
// now after binding we have to create a

// BACKLOG QUEUE

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
int main() {
    int fd = socket(AF_INET , SOCK_STREAM , 0);
    if(fd == -1) return 1;
    struct sockaddr_in addr ;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    int skt = bind(fd , (struct sockaddr*)&addr , sizeof(addr));
    if(skt < 0) return 1;
    int lst = listen(fd , 10);
    if(lst < 0) {
        std::cout << "Not listening" << std::endl;
    } 
    else {
        std::cout << "Listening" << std::endl;
    }
    return 0 ;
}