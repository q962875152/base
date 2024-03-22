#ifndef RING_BUFFER_HH_
#define RING_BUFFER_HH_

#include <memory>

template<typename T>
class RingBuffer {
public:
    
private:
    std::unique_ptr<T>* ptr;
};

#endif