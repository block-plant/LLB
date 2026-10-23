// The CRLF Trap (\r\n)
// You have parsed the Request Line 
// (Method and Route). Now you need 
// to parse the HTTP Headers.

// HTTP was designed in the 1990s, 
// heavily influenced by physical typewriters. 
// Because of this, lines in an HTTP request do 
// not end with a simple \n (newline). 
// They strictly end with CRLF: \r\n (Carriage Return + Line Feed).

// To extract the headers, we have to find these \r\n boundaries 
// and slide our window down one line at a time.

#include <cstddef>
#include <iostream>
#include <string_view>

int main() {
    const char* buffer = "GET /api/users HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string_view window(buffer);
    size_t i = window.find(" ");
    std::string_view method = window.substr(0 , i);
    std::cout << method << std::endl;
    size_t start = i + 1 ;
    size_t j = window.find(" " , start);
    std::string_view route = window.substr(start , j - start);
    std::cout << route << std::endl;
    size_t k = window.find("\r\n");
    size_t start_Header = k + 2 ; // because \r\n is 2 characters !!
    size_t l = window.find("\r\n" , start_Header);
    std::string_view header = window.substr(start_Header , l - start_Header);
    std::cout << header << std::endl;
    return 0;
}