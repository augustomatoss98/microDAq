#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stddef.h>

/** @brief A circular buffer implementation
 * 
 * This class implements a generic ring buffer that can hold a fixed number
 * of elements of any type. It provides methods to push, pop and peek data
 * has flags to check if the buffer is full or empty, and to flush the buffer.
 * 
 * The buffer is implemented using a fixed-size array and two indices (head 
 * and tail) to keep track of the start and end of the data.
 */

template<typename T, size_t size>
class RingBuffer{
public:
    /** @brief Constructor for the RingBuffer class
     * 
     * Initializes the head and tail indices to zero and the element counter to 
     * zero as well, setting the buffer to an empty state and ready for use.
     * 
     * The class is designed over a template, allowing it to store any kind of
     * data type, and customizable size defined at compile time.
     */
    RingBuffer();

    /** @brief Pushes an element into the ring buffer
     * 
     * If the buffer is not full, this function adds the specified element to the
     * buffer and updates the head index. Otherwise, it returns false.
     * 
     * @param data The element to be pushed into the buffer
     * @return true if the element was successfully pushed, false otherwise
     */
    bool push(T data);

    /** @brief Removes an element from the ring buffer
     * 
     * If the buffer is not empty, this function removes the element at the tail
     * index and updates the tail index. Otherwise, it returns false.
     * 
     * @param data The variable to store the removed element
     * @return true if an element was successfully removed, false otherwise
     */
    bool pop(T& data);

    /** 
     * @brief Flushes the ring buffer
     * 
     * This function clears the buffer by resetting the head and tail indices to zero
     * and setting the element counter to zero.
     * 
     * @return true if the buffer was successfully flushed, false otherwise
     */
    bool flush();
    /** 
     * @brief Peeks at an element at a specific index
     * 
     * This function retrieves the element at the specified index without removing it.
     * 
     * @param idx The index of the element to peek at
     * @param data The variable to store the retrieved element
     * @return true if the element was successfully retrieved, false otherwise
     */
    bool peek_at(size_t idx, T& data);

    /**
     * @brief Checks if the ring buffer is full
     * 
     * This function returns true if the buffer is full, false otherwise.
     * 
     * @return true if the buffer is full, false otherwise
     */
    bool is_full() const;

    /** 
     * @brief Checks if the ring buffer is empty
     * 
     * This function returns true if the buffer is empty, false otherwise.
     * 
     * @return true if the buffer is empty, false otherwise
     */
    bool is_empty() const;


private:
    size_t head;    // Write index
    size_t tail;    // Read index
    T buffer[size]; // Storage array
    size_t count;   // Element counter
};

#include "RingBuffer.tpp"
#endif