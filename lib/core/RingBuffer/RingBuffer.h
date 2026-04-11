#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stddef.h>

template<typename T, size_t size>
class RingBuffer{
public:
    RingBuffer();
    bool push(T data);
    bool pop(T& data);
    bool flush();
    bool peek_at(size_t idx, T& data);

    bool is_full() const;
    bool is_empty() const;


private:
    size_t head;
    size_t tail;
    T buffer[size];
    size_t count;
};

#include "RingBuffer.tpp"
#endif