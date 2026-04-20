#include <unity.h>
#include "crc/crc.h"

// Reset the state before each test
void setUp(){};

// Clean up after each test
void tearDown(){};

/**
 * Tests the CRC8 function with an empty input
 * 
 * Assertions:
 * 1. Checks that the CRC of an empty buffer is 0x00 as expected.
 */
void test_crc_empty(){
    uint8_t data[] = {};
    TEST_ASSERT_EQUAL_UINT8(0x00, crc8(data, 0));
}

/**
 * Tests the CRC8 function with a known input
 * 
 * Assertions:
 * 1. Validates that the CRC8 of the input "Hi!" matches the expected value 
 * (0x78).
 */
void test_crc(){
    uint8_t data[] = {0x48, 0x69, 0x21};
    TEST_ASSERT_EQUAL_UINT8(0x78, crc8(data, 3));
}

/**
 * Tests the consistency of the CRC8 function
 * 
 * Assertions:
 * 1. Checks that different data arrays produce different CRC values.
 */
void test_consistency(){
    uint8_t data1[] = {0x01, 0x02};
    uint8_t data2[] = {0x01, 0x02, 0x03};

    uint8_t crc1 = crc8(data1, 2);
    uint8_t crc2 = crc8(data2, 3);

    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

/**
 * Tests the sensitivity of the CRC8 function to bit changes
 * 
 * Assertions:
 * 1. Validates that flipping a single bit in the input data results different
 * CRC values.
 */
void test_bit_flip(){
    uint8_t data1[] = {0xAA};
    uint8_t data2[] = {0xAB};

    uint8_t crc1 = crc8(data1, 1);
    uint8_t crc2 = crc8(data2, 1);

    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

/**
 * Tests the CRC8 function with a larger buffer
 * 
 * Assertions:
 * 1. Checks that the CRC of a larger buffer is not 0x00.
 */
void test_crc_buffer(){
    uint8_t data[32];

    for (int i = 0; i < 32; i++)
        data[i] = i;

    uint8_t crc1 = crc8(data, 32);

    TEST_ASSERT_NOT_EQUAL(0x00, crc1);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_crc_empty);
    RUN_TEST(test_crc);
    RUN_TEST(test_consistency);
    RUN_TEST(test_bit_flip);
    RUN_TEST(test_crc_buffer);
    
    return UNITY_END();
}