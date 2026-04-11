#include "RingBuffer.h"

template<typename T, size_t size>
RingBuffer<T,size>::RingBuffer(){
    this->tail = 0;
    this->head = 0;
    this->count = 0;
}

template<typename T, size_t size>
bool RingBuffer<T, size>::push(T data){
    if(this->is_full()) return false;

    size_t next = (this->head + 1) % size;
    this->buffer[head] = data;
    head = next;
    count++;
    
    return true;
}

template<typename T, size_t size>
bool RingBuffer<T, size>::pop(T& data){
    if(this->is_empty()) return false;

    data = buffer[tail];
    tail = (tail + 1) % size;
    count--;
    return true;
}

template<typename T, size_t size>
bool RingBuffer<T,size>::flush(){
    if(this->is_empty()) return false;

    head = tail = 0;
    count = 0;
    return true;
}

template<typename T, size_t size>
bool RingBuffer<T,size>::peek_at(size_t idx, T& data){
    if(this->is_empty()) return false;

    data = buffer[idx];
    return true;
}

template<typename T, size_t size>
bool RingBuffer<T, size>::is_full() const{
    return  this->count == size;

}

template<typename T, size_t size>
bool RingBuffer<T,size>::is_empty() const{
    return this->count == 0;
}