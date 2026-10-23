// in last lesson we looped and found \r\n , but we can have 
// many header. we need to automate it , and not hardcode it.

// The Header Parsing Loop
// A real web browser sends 15 to 20 headers, 
// not just one. You cannot hardcode first_crlf, 
// second_crlf, etc. You need a loop that slides 
// the window down line by line.

// The End of the Headers:
// How does the server know when the headers stop and 
// the actual body (like a JSON payload or a file upload) 
// begins?
// The HTTP protocol dictates that the headers are 
// separated from the body by a completely blank line.
// In raw bytes, that looks like a double CRLF: \r\n\r\n.

// If you slide your window down and the length of 
// the next line is exactly 0 (meaning you found a \r\n 
// immediately after another \r\n), you have reached 
// the end of the headers.

#include <cstddef>
#include <iostream>
#include <string_view>

int main() {
    const char* buffer = "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl\r\nAccept: */*\r\n\r\n";
    std::string_view window(buffer);
    size_t i = window.find(" ");
    std::string_view method = window.substr(0 , i);
    std::cout << method << std::endl;
    // we dont have route here so we are not extracting that 
    // finding the first \r\n 
    size_t j = window.find("\r\n");
    size_t start = j + 2 ; // they are 2 characters 
    while(start <= window.length()) {
        size_t end = window.find("\r\n" , start);
        std::string_view cur_header = window.substr(start , end - start);
        if(cur_header.length() == 0) {
            std::cout << "HEADER FINISHED !" << std::endl;
            break ; 
        }
        std::cout << cur_header << std::endl;
        start = end + 2 ;
    }
    return 0 ; 
}