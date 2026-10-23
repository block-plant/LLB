// okay so until now , we first made linear arena
// then we found the padding issue (hardware problem)
// then we fixed that using an external function 
// then we got a problem of not being able to delete the allocated space
// so we made a linked list type of structure that is based on 
// analogy of a parking lot.

// now lets start out lesson4 !!!

// mixing up Arena with freeing up of custom space !

#include <iostream>
#include <cstddef>
#include <new>
#include <memory>

size_t fixAlignment(const size_t alignment , const size_t offset) {
    if(offset % alignment == 0) return offset ;
    size_t newOffset = ((offset / alignment) + 1) * alignment ; 
    return newOffset ;
}
class Arena {
    private:
        char* buffer ; 
        size_t capacity ;
        size_t offset ; 
    public:
        Arena(const size_t n) : capacity(n) , offset(0) {
            buffer = new char[n];
        }
        ~Arena() {
            delete[] buffer; 
        }
        void* allocate(const size_t need , const size_t alignment=8) {
            size_t newBoundary = fixAlignment(8 , offset);
            if(need + newBoundary <= capacity) {
                void* empty = buffer + newBoundary;
                offset = need + newBoundary;
                return empty;
            }
            return nullptr;
        }
        void freeToMarker(const size_t position) {
            offset = position ; // all the memory ahead of position will be lost 
        }
        size_t getMarker() {
            return offset ; 
        }
        void reset() {
            offset = 0 ;
        }
};
int main() {
    Arena myStack(1024);
    void* land = myStack.allocate(sizeof(int));
    int* ptr = new (land) int(12);
    size_t savePoint = myStack.getMarker();
    std::cout << savePoint << std::endl;
    void* secondLand = myStack.allocate(sizeof(float));
    float* ftr = new (secondLand) float(0.86);
    std::cout << ftr << std::endl;
    myStack.freeToMarker(savePoint);
    void* thirdLand = myStack.allocate(sizeof(double));
    double* dtr = new (thirdLand) double(124);
    std::cout << dtr << std::endl;
    size_t newCheckPoint = myStack.getMarker();
    std::cout << newCheckPoint << std::endl;
    return 0 ; 
}
