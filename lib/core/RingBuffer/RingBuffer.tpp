#include "RingBuffer.h"


template<typename T, size_t size>
RingBuffer<T,size>::RingBuffer(){
    this->tail = 0;
    this->head = 0;
    this->count = 0;
}


template<typename T, size_t size>
bool RingBuffer<T, size>::push(T data){
    if(this->is_full()) return false;   // Buffer is full, cannot push new data

    // Add new data at the head index and update head and count
    size_t next = (this->head + 1) % size;
    this->buffer[head] = data;
    head = next;
    count++;
    
    return true;
}


template<typename T, size_t size>
bool RingBuffer<T, size>::pop(T& data){
    if(this->is_empty()) return false;  // Buffer is empty, cannot pop data
    
    // Retrieve data at the tail index and update tail and count
    data = buffer[tail];
    tail = (tail + 1) % size;
    count--;
    return true;
}


template<typename T, size_t size>
bool RingBuffer<T,size>::flush(){
    if(this->is_empty()) return false; // Buffer is already empty, nothing to flush

    // Reset head and tail indices and count to clear the buffer
    head = tail = 0;
    count = 0;
    return true;
}


template<typename T, size_t size>
bool RingBuffer<T,size>::peek_at(size_t idx, T& data){
    if(this->is_empty()) return false;  // Buffer is empty, cannot peek data

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