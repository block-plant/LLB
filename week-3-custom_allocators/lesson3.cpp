// this is the lesson 3 
// in last class we implemented our linear arena exactly as the hardware
// shows up , space + padding( if required )

// The Object Pool (Free Lists)
// Theory: The Linear Arena's Fatal Flaw
// Your Linear Arena is blistering fast, 
// but it has one strict limitation: you 
// cannot free individual objects.
// If you allocate 10,000 network connections 
// and connection #5 closes, you cannot reuse 
// that memory until the entire Arena is reset. 
// The memory is trapped.

// Core Intuition: The Parking Lot
// Imagine a parking lot strictly for identical delivery vans.
// Because every van is the exact same size, 
// you don't need a sliding offset counter. 
// You just put an orange traffic cone in every empty space.

// When a van arrives, 
// you take a cone away and park it.

// When a van leaves, 
// you put a cone back in that exact spot. 
// The next van can park right there.

// In systems engineering, 
// this is called an Object Pool or a Free List. 
// It manages a block of memory divided into perfectly 
// equal-sized chunks (e.g., fixed-size network packets or database rows). 
// All empty chunks are linked together in a Linked List.


// ObjectPool implementation 

#include <iostream>
#include <new>
#include <memory>
struct Freenode {
    Freenode *next ;
};
class ObjectPool {
    private:
        char* buffer ;
        Freenode* head ;
    public: 
        ObjectPool(const size_t chunks , const size_t count) {
            buffer = new char[chunks * count];
            for(size_t i = 0 ; i < count ; i++) {
                char* currentAddr = buffer + (i * chunks) ;
                Freenode* currentPtr = (Freenode*)currentAddr ; 
                if(i == count - 1) {
                    currentPtr->next = nullptr ;  
                }
                else {
                    // getting the next addr ; 
                    char* nextAddr = buffer + ((i + 1) * chunks);
                    currentPtr->next = (Freenode*)nextAddr ; 
                }
            }
            head = (Freenode*)buffer ;
        }
        void* allocate() {
            if(head != NULL) {
                Freenode* temp = head ; 
                head = head->next ;
                return temp ; 
            }
            return nullptr ; 
        }
        void deallocate(void* ptr) {
            Freenode* temp = (Freenode*)ptr ;
            temp->next = head; 
            head = temp ; 
        }
};
int main() {
    ObjectPool op(32 , 5);
    void* empty = op.allocate();
    int* myPtr = new (empty) int(20);
    std::cout << myPtr << std::endl;
    void* et = op.allocate();
    float* In = new (et) float(300.012);
    std::cout << In << std::endl;
    op.deallocate(empty) ;
    void* emp = op.allocate();
    int* pt = new (emp) int(20);
    std::cout << pt << std::endl;
}


// The Parking Lot Analogy:
// The Linked List is just a rope connecting 
// all the orange traffic cones.
// When a van parks, you pick up the cone and 
// cut it off the rope.
// When the van leaves, you don't try to 
// tie the cone back into its original spot 
// in the middle of the rope. You just tie 
// it to the very front of the rope and say, 
// "This is the newest empty spot."