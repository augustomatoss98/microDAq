#include <unity.h>
#include "RingBuffer.h"

RingBuffer<uint8_t, 4> buffer;

void setUp(){
    buffer.flush();
};

void tearDown(){};

void test_buffer_is_empty(){
    TEST_ASSERT_TRUE(buffer.is_empty());
};

void test_buffer_is_full(){
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);
    
    TEST_ASSERT_FALSE(buffer.push(5));
    TEST_ASSERT_TRUE(buffer.is_full());
};

void test_peek_at(){
    uint8_t x;
    
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);

    buffer.peek_at(2, x);

    TEST_ASSERT_TRUE(x == 3 && buffer.is_full());
}

void test_flush(){
    buffer.push(1);
    buffer.push(2);
    buffer.push(3);
    buffer.push(4);

    buffer.flush();

    TEST_ASSERT_TRUE(buffer.is_empty());
}

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