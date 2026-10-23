// in this we will be moving from arrays to generalized single-object 
// wrappers.

// Concept: Operator Overloading and Templates
// To make a class behave like a raw pointer, 
// you must implement the dereference operator (*) 
// and the arrow operator (->). 
// You also need to make the class generic using template <typename T>.

// in last class we made buffer int , but what if we need float
// we will need to make another class BufferFloat 
// so in order to avoid that we use template <typename T> ,
// this waits for the user to define what it needs so it simply becomes that.


#include <iostream>
// template<typename T>
// class Buffer {
//     private:
//         T *data;
//     public:
//         Buffer(T *ptr) {
//             std::cout << *ptr << std::endl;
//             data = ptr ;
//         }
//         ~Buffer() {
//             delete data ;
//         } 
// };
// int main() {
//     Buffer<int> myBuf(new int(4));
//     Buffer<float> myB(new float(0.4));
//     return 0 ;
// }

// Concept 2: Operator Overloading (* and ->)
// If you have a raw pointer, 
// you access its value using the dereference 
// operator (*ptr). If that pointer points to a struct or class, 
// you access its members using the arrow operator (ptr->method()).
#include <cstddef>

template <typename T>
class UniquePtr {
    private:
        T*ptr;
    public:
        UniquePtr(T* other) {
            ptr = other;
        }
        ~UniquePtr() {
            delete ptr;
        }
        UniquePtr(const UniquePtr&) = delete;
        UniquePtr& operator=(const UniquePtr&) = delete;
        UniquePtr(UniquePtr&&other) noexcept {
            ptr = other.ptr;
            other.ptr = nullptr ;
        }
        UniquePtr& operator=(UniquePtr&&other) noexcept {
            if(this != &other) {
                delete ptr;
                ptr = other.ptr ;
                other.ptr = nullptr ;
            }
            return *this;
        }
        T& operator*() const {
            return *ptr;
        }
        T* operator->() const {
            return ptr;
        }
};
int main() {
    UniquePtr<int> intPtr(new int(42));
    std::cout << "Value: " << *intPtr << std::endl;
    UniquePtr<std::string> strPtr(new std::string("Systems"));
    std::cout << "Length: " << strPtr->length() << std::endl;
    UniquePtr<int> stolen = std::move(intPtr);
    std::cout << "Stolen Value: " << *stolen << std::endl;
    return 0 ; 
}
