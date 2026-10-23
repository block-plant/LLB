// When a web browser connects to your raw TCP server, 
// the recv() buffer fills with raw ASCII bytes formatted exactly like this:


// GET /api/users HTTP/1.1\r\n
// Host: localhost:8080\r\n
// User-Agent: Mozilla/5.0\r\n
// \r\n

// To figure out what the user wants, 
// your server needs to extract the Method (GET) 
// and the Route (/api/users).

// The Heap Allocation Trap
// In standard C++, developers parse text by 
// calling std::string::substr(). Every time you call substr(), 
// the standard library asks the OS for new heap memory and copies 
// the characters over. If you parse 10 headers on 10,000 concurrent 
// connections, you trigger 100,000 heap allocations per second.
// The CPU spends all its time managing memory fragmentation 
// instead of serving users.

// The Zero-Cost Window: std::string_view
// Introduced in C++17, std::string_view 
// is just a lightweight struct containing two things: 
// a pointer (const char*) and a length (size_t).
// It does not own memory and never copies bytes. 
// It just drops a physical "window" over the existing 
// char buffer you already populated during the recv() system call.

// DAY 1 : EXTRACTING THE HTTP METHOD 

// Your first task is to take a raw hardcoded HTTP string, 
// wrap it in a string_view, and extract the GET method 
// without allocating a single byte of new memory.

#include <iostream>
#include <string_view>

int main() {
    // taking the data that is coming through recv()
    const char* raw_buffer = "GET /api/users HTTP/1.1\r\nHost: localhost\r\n\r\n" ; 
    std::string_view request(raw_buffer) ;
    // now we need to find from this string that it is GET 
    // METHOD 1 :
    int i = request.find(" ");
    std::cout << "METHOD 1 : " << i << std::endl;
    // METHOD 2 : 
    std::string_view str = request.substr(0 , i);
    // Note: string_view's substr() does NOT allocate memory! 
    // It just returns a smaller window.
    std::cout << str << std::endl;
    return 0;
}

// Your extraction is perfect. Under the hood, 
// str is literally just a struct holding a pointer 
// to the 'G' and the integer 3. 
// The OS heap manager is completely bypassed.