// here we are starting systems engineering !!!



// size_t 
// When you define the capacity of an array or a memory buffer, 
// never use a standard int. You use size_t. 
// It is an unsigned integer type guaranteed by 
// the C++ standard to be large enough to represent 
// the size of the largest possible object in memory. 
// On a 64-bit architecture, size_t is a 64-bit unsigned integer.


// new[] 
// Allocating a single integer looks like this: int* ptr = new int(5);
// Allocating a contiguous block of integers requires the bracket syntax:
// int* arr = new int[1024];.
// Under the hood, 
// the runtime asks the OS to find a contiguous block of heap memory
// large enough to hold 1024 integers (4096 bytes) and returns the 
// memory address of the first element.


// delete[]
// This is the most common source of memory corruption for C++ beginners.
// If you allocate with new[], you must strictly free with delete[].
// If you write delete arr; on an array, 
// the compiler only calls the destructor for the first element. 
// The rest of the block remains locked, causing a massive memory leak. 
// When you write delete[] arr;, the runtime reads a hidden metadata tag 
// (usually stored exactly one word before the array pointer) 
// to find out how many elements exist, destroys all of them, 
// and frees the entire block.


// Preventing the Double-Free Crash (The Rule of Three)
// If you assign one Buffer object to another (Buffer b2 = b1;), 
// C++ automatically generates a shallow copy. Both b1 and b2 
// now hold a pointer to the exact same heap address.
// When b2 goes out of scope, its destructor calls delete[] 
// and frees the memory. When b1 goes out of scope immediately after, 
// its destructor calls delete[] on that exact same address. 
// The OS catches this "double-free" and forcefully terminates 
// your program with a segmentation fault.

// we do two things to ensure that copying is not possible 
// Buffer(const Buffer&) = delete ; // stops copying the constructor
// Buffer b = a ; this cant be done 
// Buffer& operator=(const Buffer&) delete ; // stop copying the assignment 
// b = a ; this cant be done 


// homework :

#include <iostream>
#include <cstddef>
class Buffer {
    private: 
        size_t size ; 
        int* ptr; 
    public: 
        Buffer(const Buffer&) = delete ;
        Buffer& operator=(const Buffer&) = delete;
        Buffer(const size_t sz) : size(sz) {
            ptr = new int[size];
            std::cout << "Creating the Buffer" << std::endl;
        }

        ~Buffer() {
            delete[] ptr;
            std::cout << "Buffer Destroyed" << std::endl;
        } 
};
int main() {
    {
        Buffer myBuf(1024);
    }
    return 0 ;
}


// this is the implementation for RAII ( Resource Acquistion Is Initialization )
// second unsaid rule is Resource Release is Destruction.