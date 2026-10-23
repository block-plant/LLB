// this is day 5 , until now we learnt about parsing all headers 

// Key-Value Header Splitting
// Right now, you are extracting the entire line: Host: localhost.
// For a backend engine, this is useless. You need to check specific 
// keys, like Content-Length to know how many bytes of JSON the user 
// uploaded, or Authorization to check their API token.

// You must split every header into a Key and a Value.

// The Delimiter:
// HTTP headers are strictly separated by a colon: :.
// There is almost always a space after the colon, 
// but the protocol does not guarantee it.

// The remove_prefix Weapon:
// Because std::string_view is just a pointer and a length, 
// C++ gives you a way to shrink the window from the front 
// in O(1) time without calling substr().

// Calling value.remove_prefix(1) physically moves the 
// starting pointer forward by one byte and decreases the 
// length by 1. It is the fastest way to trim leading whitespace.
  
// bringing the standar code from last class 
// in this class we just want to write the headers as key : value pair

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
        // finding the colon to differentiate between key value 
        size_t colon_idx = cur_header.find(":");
        if(colon_idx != std::string_view::npos) {
            std::string_view key = cur_header.substr(0 , colon_idx);
            std::string_view value = cur_header.substr(colon_idx + 1);
            if(value.length() > 0 && value[0] == ' ') {
                value.remove_prefix(1); // this just removes the first byte without any overhead.
            }
            std::cout << key << " : " << value << std::endl;
        }
        start = end + 2 ;
    }
    return 0 ; 
}
