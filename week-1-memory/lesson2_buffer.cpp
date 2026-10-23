// in this we are going to learn about MOVE semantics 


// We disabled copying, 
// which is good for safety, 
// but bad for flexibility. 
// What if you want to create a Buffer inside a function 
// and return it? Returning by value would attempt to trigger 
// the deleted copy constructor.

// C++11 solved this with Move Semantics.
// implementing the move 
#include <iostream>
#include <cstddef>
// class SafeInt {
//     private:
//         int* data;
//     public:
//         SafeInt(const int value) {
//             data = new int(value);
//         }
//         ~SafeInt() {
//             delete data;
//         }
//         // deleting the copying thing 
//         SafeInt(const SafeInt&) = delete ;
//         SafeInt& operator=(const SafeInt&) = delete ;
//         // now for the moving Part 
//         SafeInt(SafeInt&&other) noexcept {
//             data = other.data ;
//             other.data = nullptr ;
//         }
//         SafeInt& operator=(SafeInt&&other) noexcept {
//             if(this != &other) {
//                 delete data ;
//                 data = other.data ; 
//                 other.data = nullptr ;
//             }
//             return *this ; 
//         }
// };
// SafeInt create(int val) {
//     SafeInt temp(val);
//     return temp ; 
// }
// int main() {
//     {
//         SafeInt sf = create(42) ;
//         SafeInt b(10);
//         b = create(99);  
//     }
// }
class Buffer {
    private: 
        size_t size ; 
        int* ptr ; 
    public: 
        Buffer(const size_t sz) {
            size = sz ;
            ptr = new int[size] ; 
        }
        ~Buffer() {
            delete[] ptr ; 
        }
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete ; 
        Buffer(Buffer&&other) noexcept {
            size = other.size ;
            ptr = other.ptr ;
            other.ptr = nullptr ;
            other.size = 0 ; 
        }
        Buffer& operator=(Buffer&&other) noexcept {
            if(this != &other) {
                std::cout << "here " << std::endl;
                delete[] ptr ; 
                ptr = other.ptr ;
                size = other.size ; 
                other.ptr = nullptr;
                other.size = 0 ;
            }
            return *this ; 
        }
};
Buffer buf(size_t sz) {
    Buffer temp(sz); 
    return temp ; 
}
int main() {
    Buffer catched = buf(101);
    Buffer b(10);
    b = std::move(catched);
    return 0 ;
}