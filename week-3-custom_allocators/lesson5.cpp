// so in last class we did Arena allocator , but with variable offset erase 

// so we called it the STACK ALLOCATOR !
// now lets see what we have today !

// The Frame Allocator (Double Buffering)
// To recap the ping-pong analogy: 
// you cannot delete objects while they are actively 
// being drawn to the screen. We need two Arenas. 
// We draw with Arena A while doing background math in Arena B. 
// On the next frame, we swap them and wipe the inactive one clean.

#include <iostream>
#include <new>
#include <cstddef> 
#include <ostream>

size_t fixAlignment(const size_t alignment , const size_t offset) {
   if(offset % alignment == 0) return offset;
   return (size_t)(((offset / alignment) + 1) * alignment) ; 
}
class Arena {
    private:
        char* buffer ;
        size_t capacity;
        size_t offset; 
    public:
        Arena(const size_t sz) : capacity(sz) , offset(0) {
            buffer = new char[sz];
        }
        ~Arena() {
            delete[] buffer;
        }
        void reset() {
            offset = 0 ; 
        }
        void returnToPoint(const size_t mark) {
            offset = mark ;
        }
        size_t getMarker() {
            return offset;
        }
        void* allocate(const size_t need , const size_t alignment=8) {
            size_t newBoundary = fixAlignment(alignment , offset);
            if(need + newBoundary <= capacity) {
                void* empty = buffer + newBoundary ;
                offset = need + newBoundary;
                return empty;
            }
            return nullptr;
        }
};
class FrameAllocator {
    private:
        Arena arenaA; 
        Arena arenaB;
        bool useA;
    public:
        FrameAllocator(const size_t capacity) :
            arenaA(capacity),
            arenaB(capacity),
            useA(true) {}
        ~FrameAllocator() {}
        void swap() {
            useA = !useA;
            if(useA) {
                arenaA.reset() ; 
            }
            else {
                arenaB.reset() ;
            }
        }
        void* allocate(const size_t need) {
            void *empty = nullptr ;
            if(useA) {
                empty = arenaA.allocate(need);
            }
            else {
                empty = arenaB.allocate(need);
            }
            return empty;
        }
};
int main() {
    FrameAllocator engine(1024);
    void* land1 = engine.allocate(sizeof(int));
    int* ptr = new (land1) int(243);
    std::cout << land1 << std::endl;
    engine.swap() ;
    void* land2 = engine.allocate(sizeof(float));
    float* ftr = new(land2) float(8.99);
    std::cout << land2 << std::endl;
    engine.swap();
    void* land3 = engine.allocate(sizeof(double));
    double* dtr = new (land3) double(484236);
    std::cout << land3 << std::endl;
    return 0 ;
}