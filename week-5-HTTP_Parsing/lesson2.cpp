// in the previous lesson we learnt to extract the method 
// and now we are trying to find the route /api/route

// Now that you have the Method, 
// you need to slide the window forward to grab the 
// Route (/api/users).

// Sliding the Window (The Route)
// To find the route, you need to find the second space in the string.

// Two key mechanics of std::string_view:

// Targeted Search: The find() function takes an optional second argument: 
// where to start looking.
// request.find(' ', start_index) will skip the first space and find the next one.

// Length, not End Index: The substr(start, length) function takes the 
// starting position and the number of characters to include, not the 
// ending position. To calculate the length of the route, subtract the 
// starting index from the ending index.

#include <iostream>
#include <string_view>

int main() {
    const char* raw_buffer = "GET /api/users HTTP/1.1\r\nHost: localhost\r\n\r\n" ;     
    // first extract GET 
    std::string_view str(raw_buffer);
    int i = str.find(" "); // this will find the first index where we have ' '
    std::cout << i << std::endl;
    std::string_view method = str.substr(0 , i); // this will get the method (GET)
    int j = str.find(" " , i + 1); // this will get me the index with space but after the first one
    std::cout << j << std::endl;
    int len = j - (i + 1) ; 
    std::string_view route = str.substr(i + 1 , len);
    std::cout << route << std::endl; // this will get the me route (/api/users)
    return 0 ; 
}

// always use size_t instead of int for string positions.