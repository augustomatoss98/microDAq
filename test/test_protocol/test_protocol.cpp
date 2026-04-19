#include <unity.h>
#include <cstdio>
#include <algorithm>
#include <vector>
#include "Protocol/Protocol.h"

Protocol protocol;
static std::vector<uint8_t> tx_buffer;
static std::vector<std::vector<uint8_t>> tx_frames;

static uint32_t fake_time = 0;

uint32_t SystemTime::millis(){
    return fake_time;
}

void advance(uint32_t ms){
    fake_time += ms;
}

void reset(){
    fake_time = 0;
}


void print_frames(){
    for (size_t i = 0; i < tx_frames.size(); i++){
        printf("Frame %d:", i);
        for(uint8_t byte : tx_frames[i]){
            printf(" %02x", byte);
        }
        printf("\n");
    }
}

void write_mock(uint8_t b) {
    tx_buffer.push_back(b);

    // detectar fim de frame (ETX = 0x03)
    if (b == 0x03) {
        tx_frames.push_back(tx_buffer);
        tx_buffer.clear();
    }
}

void feed_bytes(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++){
        protocol.process(data[i]);
    }
}


void setUp(){
    protocol.set_write_callback(write_mock);
    protocol.flush_rx_queue();
    protocol.tx = Protocol::PendingTx{};
    tx_buffer.clear();
    tx_frames.clear();
    reset();
};

void tearDown(){};

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

    TEST_ASSERT_FALSE(protocol.tx.waiting_ack);

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

    for (size_t i = 0; i < tx_frames.size(); i ++) {
        for (uint8_t byte : tx_frames[i]){
            if (byte == static_cast<uint8_t>(Protocol::Command::ACK)) {
            ack_count++;
            }   
        }
    }

    TEST_ASSERT_EQUAL_UINT8(pkt_count, 5);
    TEST_ASSERT_EQUAL(ack_count, 10);
}


void test_retry_limit(){
    protocol.send_command(Protocol::Command::PING, nullptr, 0);

    TEST_ASSERT_EQUAL(1, tx_frames.size());

    advance(Protocol::ACK_TIMEOUT_MS);
    protocol.update();

    TEST_ASSERT_EQUAL(2, tx_frames.size());
    TEST_ASSERT_EQUAL(1, protocol.tx.retry_count);

    advance(Protocol::ACK_TIMEOUT_MS);
    protocol.update();

    TEST_ASSERT_EQUAL(3, tx_frames.size());
    TEST_ASSERT_EQUAL(2, protocol.tx.retry_count);

    advance(Protocol::ACK_TIMEOUT_MS);
    protocol.update();

    TEST_ASSERT_EQUAL(3, tx_frames.size());
    TEST_ASSERT_FALSE(protocol.tx.waiting_ack);
}


void test_retry_same_frame() {
    protocol.send_command(Protocol::Command::PING, nullptr, 0);

    advance(Protocol::ACK_TIMEOUT_MS);
    protocol.update();

    advance(Protocol::ACK_TIMEOUT_MS);
    protocol.update();

    // compara frame 0, 1 e 2
    TEST_ASSERT_EQUAL_MEMORY(
        tx_frames[0].data(), tx_frames[1].data(), tx_frames[0].size()
    );

    TEST_ASSERT_EQUAL_MEMORY(
        tx_frames[1].data(), tx_frames[2].data(), tx_frames[1].size()
    );
}


void test_ack_stops_retry() {
    protocol.send_command(Protocol::Command::PING, nullptr, 0);

    uint8_t frame[] = {
        0x02,
        0x00,
        0x01,
        0xF0,
        0x00,
        0x03
    };

    frame[2] = protocol.tx.seq;
    frame[4] = crc8(&frame[1], 3);
    feed_bytes(frame, sizeof(frame));

    // ⏱️ avança tempo
    advance(Protocol::ACK_TIMEOUT_MS);
    protocol.update();

    // ❗ não deve retransmitir
    TEST_ASSERT_EQUAL(1, tx_frames.size());
    TEST_ASSERT_FALSE(protocol.tx.waiting_ack);
}

void test_nack_triggers_retry() {
    protocol.send_command(Protocol::Command::PING, nullptr, 0);

    uint8_t frame[] = {
        0x02,
        0x00,
        0x01,
        0xF0,
        0x00,
        0x03
    };

    frame[2] = protocol.tx.seq;
    frame[4] = crc8(&frame[1], 3);
    feed_bytes(frame, sizeof(frame));    

    protocol.update();

    // retry imediato
    TEST_ASSERT_EQUAL(2, tx_frames.size());
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
    RUN_TEST(test_retry_limit);
    RUN_TEST(test_retry_same_frame);
    RUN_TEST(test_ack_stops_retry);
    RUN_TEST(test_nack_triggers_retry);
    
    return UNITY_END();
}