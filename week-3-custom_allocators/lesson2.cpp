// in last class we did allocators and constructors 
// and then we build our own linear arena

// in last class we had a doubt about the memory alignment !! 

// TODAY'S LECTURE : MEMORY ALIGNMENT

// class Something {
//     private: 
//         int a ; 
//         char b ; 
//         std::vector<int> d ; // this calls new under the hoop , which does heap fragmentation storage 
//         int size ; 
//     public: 
//         Something(const int sz) : size(sz) {
//             d.resize(sz) ; 
//             a = 1;  
//             b = 10;
//         }
//         ~Something() {}
// };

// If the 10 integers aren't inside the class, 
// what takes up 40 bytes?

// int a: 4 bytes
// char b: 1 byte

// std::vector d: 24 bytes 
// (A vector is literally just 3 memory pointers: 
// begin, end, and capacity. 
// 3 pointers x 8 bytes = 24).

// int size: 4 bytes

// Total: 33 bytes.

// Why did C++ say 40? Because of the CPU hardware.

// Your M1 Apple Silicon chip reads memory in chunks of 8 bytes 
// (a "Word"). If a variable starts at a random odd address like 
// buffer + 33, the CPU has to do two separate memory reads and 
// stitch them together. In older processors, this caused a literal 
// hardware crash called a Bus Error. Today, it just destroys 
// performance.

// To prevent this, the C++ compiler silently injects Padding 
// (dead, unused bytes) to push variables forward so they 
// perfectly align with multiples of 8.
// 33 bytes + 7 bytes of dead padding = 40 bytes.

// Right now, your Arena is dangerous.
// If I call allocate(1) for a char, your offset becomes 1.
// If I then call allocate(sizeof(int)), you hand me buffer + 1.
// You just forced me to construct an integer on an unaligned memory 
// address. The hardware will penalize us.

// Task 3.1: The Alignment Helper
// first implementing the Arena 
// over that i will permanent patch the fix 
#include <iostream>
#include <cstddef>
#include <new>
#include <vector>
size_t fixAlignment(size_t alignment , size_t offset) {
    if(offset % alignment == 0) return offset ; 
    size_t nextPaddedAddress = ((offset / alignment) + 1) * alignment ;
    return nextPaddedAddress ; 
}
class Arena {
    private: 
        size_t offset ; 
        size_t size ; 
        char* buffer ; 
    public: 
        Arena(const size_t sz) : size(sz) {
            offset = 0 ; 
            buffer = new char[sz];
        }
        ~Arena() {
            delete[] buffer; 
        }
        void* allocate(const size_t need , const size_t alignment = 8) {
            size_t newBoundary = fixAlignment(alignment , offset) ;
            if(need + newBoundary <= size) { 
                void* empty = buffer + newBoundary ; 
                offset = newBoundary + need ; 
                return empty ; 
            }
            return nullptr ;
        }
        void reset() {
            offset = 0 ; 
        }
};
class Something {
    private: 
        int a ; 
        char b ; 
        std::vector<int> d ; // this calls new under the hoop , which does heap fragmentation storage 
        int size ; 
    public: 
        Something(const int sz) : size(sz) {
            d.resize(sz) ; 
            a = 1;  
            b = 10;
        }
        ~Something() {}
};

int main() {
    Arena myArena(1024);
    void* empty = myArena.allocate(sizeof(char));
    char* ch = new (empty) char('a');
    std::cout << (void*)ch << std::endl;
    void* emptyInt = myArena.allocate(sizeof(int));
    int* in = new (emptyInt) int(40);
    std::cout << in << std::endl;

    // now we see that just for 1 byte of char whole 8 byte is reserved 
    // and the int starts 8 bytes after that 
    return 0 ; 
}
