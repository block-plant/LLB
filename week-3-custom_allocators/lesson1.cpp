// this lesson is based on Allocation vs Construction

// Allocation: It halts your thread, traps into the OS kernel, 
// and asks the memory manager to find free RAM.

// Construction: It calls object's constructor on that memory.

// Standard new is like asking a restaurant host to seat every 
// single guest individually as they arrive. The host runs 
// around finding empty tables, taking time and seating people 
// randomly (Heap Fragmentation).

// Placement new is booking the entire empty banquet hall upfront 
// (one single OS request). As guests arrive, you point them to 
// the next available chair. It takes one nanosecond.

// goal is to physically decouple allocation from construction 

// writing down the taught implementation 
#include <iostream> 
#include <new>
// class Task {
//     private:
//         int id ;
//     public:
//         Task(const int i) : id(i) {
//             std::cout << "Task : " << i << std::endl; 
//         }
//         ~Task() {}
// };
// int main() {
//     char* arena = new char[1024] ; // using char as 1 char = 1 byte , makes calculations easy
//     Task* t1 = new (arena) Task(1) ; // if this will have add x
//     Task* t2 = new (arena + sizeof(Task)) Task(22) ; // then this will have x + 4
//     std::cout << t1 << ' ' << t2 << std::endl ;
//     t1->~Task() ;
//     t2->~Task() ;
//     return 0 ;  
// }

// HOMEWORK 
#include <cstddef>
class Arena {
    private:
        char* buffer ;
        size_t capacity ;
        size_t offset ; 
    public: 
        Arena(const size_t c) : capacity(c) {
            buffer = new char[c] ; 
            offset = 0 ; 
        }
        ~Arena() {
            std::cout << "The space got free." << std::endl;
            delete[] buffer ;
        }
        void* allocate(size_t size) {
            if(size + offset <= capacity) {
                void* empty = buffer + offset;
                offset = offset + size ;
                return empty ; 
            }
            return nullptr ; 
        }
        void reset() {
            offset = 0 ; // this means if the storage is filled then we will rewrite over that. 
        }
}; 
#include <vector>
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
    // Ask YOUR manager for 4 bytes of empty land (the size of an integer)
    void* raw_memory = myArena.allocate(sizeof(int));
    // Use Placement New to construct the integer exactly on that empty land.
    // Syntax: new (memory_address) Type(value);
    int* myInt = new (raw_memory) int(42);
    std::cout << "Value of int : " << *myInt << std::endl;
    std::cout << "Address of int : " << myInt << std::endl;
    std::cout << "Address of raw_memory : " << raw_memory << std::endl;
    void* raw_something_pointer = myArena.allocate(sizeof(Something));
    std::cout << sizeof(Something) << std::endl;
    Something* newobj = new (raw_something_pointer) Something(10);
    std::cout << newobj << std::endl; 
    void* raw_pointer_for_check = myArena.allocate(sizeof(int));
    int* Int = new (raw_pointer_for_check) int(110); 
    std::cout << Int << std::endl; 
    return 0 ; 
}

// QUESTION : why is sizeof(Something) = 40 , even if i change it to (202) it still stays 40.
// we will see this in the next class. 

// WHAT WE BUILT HERE IS CALLED LINEAR ARENA 
// THIS ALLOCATES SPACE WITHOUT ANY OVERHEAD IN O(1).