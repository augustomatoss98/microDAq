#include <unity.h>
#include "RingBuffer/RingBuffer.h"

RingBuffer<uint8_t, 4> buffer;

// Reset the state before each test
void setUp(){
    buffer.flush();
};

// Clean up after each test
void tearDown(){};

/**
 * Tests that a newly initialized ring buffer is empty
 * 
 * Assertions:
 * 1. Checks that the ring buffer is empty immediately after initialization.
 */
void test_buffer_is_empty(){
    TEST_ASSERT_TRUE(buffer.is_empty());
};

/**
 * Tests that the ring buffer correctly identifies when it is full
 * 
 * Assertions:
 * 1. Checks if the buffer can't accept new elements when it is full.
 * 2. Validates that the is_full() method returns true when the buffer is full
 */
void test_buffer_is_full(){
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);
    
    TEST_ASSERT_FALSE(buffer.push(5));
    TEST_ASSERT_TRUE(buffer.is_full());
};

/**
 * Tests the peek_at() function of the ring buffer
 * 
 * Assertions:
 * 1. Checks that peek_at() retrieves the correct element at the specified index
 */
void test_peek_at(){
    uint8_t x;
    
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);

    buffer.peek_at(2, x);

    TEST_ASSERT_TRUE(x == 3 && buffer.is_full());
}

/**
 * Tests the flush() function of the ring buffer
 * 
 * Assertions:
 * 1. Checks that flush() empties the ring buffer
 */
void test_flush(){
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);

    buffer.flush();

    TEST_ASSERT_TRUE(buffer.is_empty());
}

/**
 * Tests the push() and pop() functions of the ring buffer
 * 
 * Assertions:
 * 1. Checks that push() adds an element to the buffer and pop() retrieves
 *  it correctly.
 */
void test_push_and_pop(){
    uint8_t x = 8; 
    uint8_t y;

    buffer.push(x);
    buffer.pop(y);

    TEST_ASSERT_TRUE(y == 8);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_buffer_is_empty);
    RUN_TEST(test_buffer_is_full);
    RUN_TEST(test_peek_at);
    RUN_TEST(test_flush);
    RUN_TEST(test_push_and_pop);
    
    return UNITY_END();
}