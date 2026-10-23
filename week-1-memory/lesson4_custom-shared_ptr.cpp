// in this we will implement shared_ptr
// Instead, it is the bridge that allows multiple 
// threads to safely look at the exact same Heap memory. 
// Without it, Thread A might finish and call delete while 
// Thread B is still reading, causing a fatal crash.

#include <iostream>
#include <string>
#include <utility>
#include <cstddef>

template <typename T>
class SharedPtr {
    private:
        size_t* ref_count ;
        T* ptr;
    public:
        SharedPtr(T* other) {
            std::cout << "Created, count is 1" << std::endl;
            ptr = other ;
            ref_count = new size_t(1); 
        } 
        ~SharedPtr() {
            --(*ref_count);
            if(*ref_count == 0) {
                delete ptr;
                delete ref_count ;
            }
        }
        // remove the copy thing
        SharedPtr(const SharedPtr&other) {
            ptr = other.ptr;
            ref_count = other.ref_count;
            ++(*ref_count);
            std::cout << *ref_count << std::endl;
        }
        SharedPtr& operator=(const SharedPtr&) = delete;
        // overriding 
        T& operator*() {
            return *ptr ; 
        }
        T* operator->() {
            return ptr ;
        }
};
int main() {
    {
        SharedPtr<int> ptr1(new int(100)); // Count starts at 1
        {
            SharedPtr<int> ptr2 = ptr1; // Copy Constructor fires. Count becomes 2.
            std::cout << "Value from ptr2: " << *ptr2 << std::endl;
        } // ptr2 goes out of scope. Destructor fires. Count drops to 1. Memory NOT deleted.
        
        std::cout << "ptr2 is dead, but ptr1 is still alive." << std::endl;
    } // ptr1 goes out of scope. Destructor fires. Count drops to 0. Memory IS deleted.
    
    return 0;
}