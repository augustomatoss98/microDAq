#include <unity.h>
#include "crc/crc.h"

void setUp(){};

void tearDown(){};

void test_crc_empty(){
    uint8_t data[] = {};
    TEST_ASSERT_EQUAL_UINT8(0x00, crc8(data, 0));
}

void test_crc(){
    uint8_t data[] = {0x48, 0x69, 0x21};
    TEST_ASSERT_EQUAL_UINT8(0x78, crc8(data, 3));
}

void test_consistency(){
    uint8_t data1[] = {0x01, 0x02};
    uint8_t data2[] = {0x01, 0x02, 0x03};

    uint8_t crc1 = crc8(data1, 2);
    uint8_t crc2 = crc8(data2, 3);

    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

void test_bit_flip(){
    uint8_t data1[] = {0xAA};
    uint8_t data2[] = {0xAB};

    uint8_t crc1 = crc8(data1, 1);
    uint8_t crc2 = crc8(data2, 1);

    TEST_ASSERT_NOT_EQUAL(crc1, crc2);
}

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