#include <unity.h>
#include <cstdio>
#include <algorithm>
#include <vector>
#include "Protocol/Protocol.h"

Protocol protocol;
std::vector<uint8_t> tx_buffer;


void write_mock(uint8_t byte){
    tx_buffer.push_back(byte);
}

void setUp(){
    protocol.set_write_callback(write_mock);
    protocol.flush_rx_queue();
    tx_buffer.clear();
};

void tearDown(){};

void feed_bytes(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++){
        protocol.process(data[i]);
    }
}


void test_valid_frame(){
    uint8_t payload[] = {0xAA, 0xBB};

    uint8_t frame[] = {
        0x02,       // STX
        0x02,       // LEN
        0x01,       // SEQ
        0x01,       // CMD (READ)
        0xAA, 0xBB, // PAYLOAD
        0x00,       // CRC
        0x03        // ETX
    };

    frame[6] = crc8(&frame[1], 5); // LEN + SEQ + CMD + PAYLOAD

    feed_bytes(frame, sizeof(frame));

    TEST_ASSERT_TRUE(protocol.available());

    auto pkt = protocol.get_packet();

    TEST_ASSERT_EQUAL_UINT8(2, pkt.len);
    TEST_ASSERT_EQUAL_UINT8(1, pkt.seq);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)Protocol::Command::READ, (uint8_t)pkt.cmd);

    TEST_ASSERT_EQUAL_UINT8(0xAA, pkt.payload[0]);
    TEST_ASSERT_EQUAL_UINT8(0xBB, pkt.payload[1]);

    TEST_ASSERT_EQUAL_UINT8((uint8_t)Protocol::Command::ACK, tx_buffer[3]);
}


void test_multiple_frames(){
    uint8_t frame1[] = {0x02,0x00,0x01,0x04,0x00,0x03};
    uint8_t frame2[] = {0x02,0x00,0x02,0x04,0x00,0x03};

    frame1[4] = crc8(&frame1[1], 3);
    frame2[4] = crc8(&frame2[1], 3);

    feed_bytes(frame1, sizeof(frame1));
    TEST_ASSERT_TRUE(protocol.available());
    protocol.get_packet();

    feed_bytes(frame2, sizeof(frame2));
    TEST_ASSERT_TRUE(protocol.available());
}


void test_resync_after_noise(){
    uint8_t noise[] = {0xFF, 0xAA, 0x00};
    
    uint8_t frame[] = {
        0x02,
        0x01,
        0x02,
        0x04, // PING
        0x55,
        0x00,
        0x03
    };

    frame[5] = crc8(&frame[1], 4);

    feed_bytes(noise, sizeof(noise));
    feed_bytes(frame, sizeof(frame));

    TEST_ASSERT_TRUE(protocol.available());
}


void test_invalid_etx(){
    uint8_t frame[]{
        0x02,
        0x01,
        0x01,
        0x01,
        0xAA,
        0x00,
        0xFF
    };

    frame[5] = crc8(&frame[1], 4);

    feed_bytes(frame, sizeof(frame));

    TEST_ASSERT_FALSE(protocol.available());
}


void test_invalid_len(){
    uint8_t frame[] = {
        0x02,
        100,
        0x01,
        0x01,
        0x03
    };

    feed_bytes(frame, sizeof(frame));

    TEST_ASSERT_FALSE(protocol.available());
}


void test_invalid_crc(){
    uint8_t frame[] = {
        0x02,
        0x01,
        0x01,
        0x01,
        0x01,
        0x00,
        0x03
    };

    feed_bytes(frame, sizeof(frame));

    TEST_ASSERT_FALSE(protocol.available());
    
    TEST_ASSERT_EQUAL_UINT8((uint8_t)Protocol::Command::NACK, tx_buffer[3]);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)Protocol::ErrorCode::INVALID_CRC, tx_buffer[4]);
}


void test_control_packet(){
uint8_t frame[] = {
        0x02,
        0x00,
        0x01,
        0xF0,
        0x00,
        0x03
    };

    frame[4] = crc8(&frame[1], 3);
    feed_bytes(frame, sizeof(frame));

    TEST_ASSERT_FALSE(protocol.pending_tx.waiting_ack);

}


void test_payload_zero(){
    uint8_t frame[] = {
        0x02,
        0x00,
        0x01,
        0x04,
        0x00,
        0x03
    };

    frame[4] = crc8(&frame[1], 3);
    feed_bytes(frame, sizeof(frame));
    
    TEST_ASSERT_TRUE(protocol.available());
    TEST_ASSERT_EQUAL_UINT8((uint8_t)Protocol::Command::ACK, tx_buffer[3]);
}


void test_buffer_overflow(){    
    uint8_t pkt_count = 0;
    uint8_t ack_count = 0;
    
    uint8_t frame[] = {
        0x02,
        0x00,
        0x01,
        0x04,
        0x00,
        0x03
    };

    for (uint8_t i = 0; i < 10; i++){
        frame[2] = i + 1;
        frame[4] = crc8(&frame[1], 3);
        feed_bytes(frame, sizeof(frame));
    }

    while(protocol.available()){
        auto pkt = protocol.get_packet();
        pkt_count++;
    }

    for (size_t i = 0; i < tx_buffer.size(); i ++) {
        if (tx_buffer[i] == static_cast<uint8_t>(Protocol::Command::ACK)) {
            ack_count++;
        }   
    }

    TEST_ASSERT_EQUAL_UINT8(pkt_count, 5);
    TEST_ASSERT_EQUAL(ack_count, 10);
}


int main() {
    UNITY_BEGIN();
    
    RUN_TEST(test_valid_frame);
    RUN_TEST(test_invalid_crc);
    RUN_TEST(test_invalid_len);
    RUN_TEST(test_invalid_etx);
    RUN_TEST(test_resync_after_noise);
    RUN_TEST(test_multiple_frames);
    RUN_TEST(test_control_packet);
    RUN_TEST(test_payload_zero);
    RUN_TEST(test_buffer_overflow);
    
    return UNITY_END();
}