#include <unity.h>
#include <cstdio>
#include <algorithm>
#include <vector>
#include "Protocol/Protocol.h"

Protocol protocol;
static std::vector<uint8_t> tx_buffer;
static std::vector<std::vector<uint8_t>> tx_frames;

// Mock time management for testing retries
static uint32_t fake_time = 0;

// Override SystemTime::millis to use fake_time
uint32_t SystemTime::millis(){
    return fake_time;
}

// Advances fake time by specified milliseconds
void advance(uint32_t ms){
    fake_time += ms;
}

// Resets fake time to zero
void reset(){
    fake_time = 0;
}

// Print transmitted frames for debugging
void print_frames(){
    for (size_t i = 0; i < tx_frames.size(); i++){
        printf("Frame %d:", i);
        for(uint8_t byte : tx_frames[i]){
            printf(" %02x", byte);
        }
        printf("\n");
    }
}

// Mock write callback to capture transmitted bytes
void write_mock(uint8_t b) {
    tx_buffer.push_back(b);

    // ETX received, store the frame and clear buffer
    if (b == 0x03) {
        tx_frames.push_back(tx_buffer);
        tx_buffer.clear();
    }
}

// Helper function to feed bytes into the protocol parser
void feed_bytes(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++){
        protocol.process(data[i]);
    }
}

// Reset state before each test
void setUp(){
    protocol.set_write_callback(write_mock);
    protocol.flush_rx_queue();
    protocol.tx = Protocol::PendingTx{};
    tx_buffer.clear();
    tx_frames.clear();
    reset();
};

// Clean up after each test
void tearDown(){};

/**
 * Tests the handling of a valid frame with payload
 * 
 * Assertions:
 * 1. Checks that a packet is available after feeding a valid frame.
 * 2. Validates that the packet fields (length, sequence, command, payload)
 *  match the expected values.
 * 3. Confirms that an ACK was sent in response to the valid frame.
 */

void test_valid_frame(){
    uint8_t payload[] = {0xAA, 0xBB};

    // Test frame
    uint8_t frame[] = {
        0x02,       // STX
        0x02,       // LEN
        0x01,       // SEQ
        0x01,       // CMD (READ)
        0xAA, 0xBB, // PAYLOAD
        0x00,       // CRC
        0x03        // ETX
    };

    // Computes CRC for the test frame
    frame[6] = crc8(&frame[1], 5); // LEN + SEQ + CMD + PAYLOAD

    // Feeds the frame into the protocol parser
    feed_bytes(frame, sizeof(frame));

    // Assertions
    TEST_ASSERT_TRUE(protocol.available());

    auto pkt = protocol.get_packet();

    TEST_ASSERT_EQUAL_UINT8(2, pkt.len);
    TEST_ASSERT_EQUAL_UINT8(1, pkt.seq);
    TEST_ASSERT_EQUAL_UINT8((uint8_t)Protocol::Command::READ, (uint8_t)pkt.cmd);

    TEST_ASSERT_EQUAL_UINT8(0xAA, pkt.payload[0]);
    TEST_ASSERT_EQUAL_UINT8(0xBB, pkt.payload[1]);

    TEST_ASSERT_EQUAL_UINT8((uint8_t)Protocol::Command::ACK, tx_buffer[3]);
}

/**
 * Tests the handling of multiple valid frames in sequence
 * 
 * Assertions:
 * 1. Checks that multiple packets are available after feeding multiple
 *  valid frames.
 */
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

/**
 * Verifies that the protocol can resynchronize after receiving noise bytes
 * before a valid frame.
 * 
 * Assertions:
 * 1. Checks that a valid packet is available after feeding noise followed by 
 *  a valid frame.
 */
void test_resync_after_noise(){
    uint8_t noise[] = {0xFF, 0xAA, 0x00};
    
    // Test frame 
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

    // Feeds noise followed by the valid frame into the protocol parser
    feed_bytes(noise, sizeof(noise));
    feed_bytes(frame, sizeof(frame));

    // Assertions
    TEST_ASSERT_TRUE(protocol.available());
}

/**
 * Tests the handling of a frame with an invalid ETX byte
 * 
 * Assertions:
 * 1. Checks that no packet is available after feeding a frame with an invalid
 *  ETX.
 */
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

/**
 * Tests the handling of a frame with an invalid length byte
 * 
 * Assertions:
 * 1. Checks that no packet is available after feeding a frame with an invalid
 * length that exceeds the maximum payload size.
 */
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

/**
 * Tests the handling of a frame with an invalid CRC byte
 * 
 * Assertions:
 * 1. Checks that no packet is available after feeding a frame with an invalid
 *  CRC.
 * 2. Checks if a NACK was sent in response to the invalid frame.
 * 3. Validates that the NACK contains the correct error code for invalid CRC.
 */
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
    TEST_ASSERT_EQUAL_UINT8((uint8_t)Protocol::ErrorCode::INVALID_CRC, 
                                tx_buffer[4]);
}

/**
 * Tests the handling of a control packet
 * 
 * Assertions:
 * 1. Checks that the protocol correctly processes a control packet (ACK) and
 * updates the pending transaction state accordingly.
 */
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

/**
 * Tests the handling of a frame with zero-length payload
 * 
 * Assertions:
 * 1. Checks that a packet is available after feeding a valid frame with zero-length
 *  payload.
 * 2. Checks if an ACK was sent in response to the valid frame with zero-length payload.
 */
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

/**
 * Tests the handling of multiple frames that exceed the receive buffer capacity
 * 
 * Assertions:
 * 1. Checks that only the maximum number of packets (5) are available after feeding
 *  multiple frames that exceed the receive buffer capacity.
 * 2. Validates that ACKs were sent for all received frames, even those that exceed
 * the buffer capacity.
 */
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

    // Count available packets
    while(protocol.available()){
        auto pkt = protocol.get_packet();
        pkt_count++;
    }

    // Count received ACKs
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

/**
 * Tests the retry mechanism of the protocol when ACKs are not received
 * 
 * Assertions:
 * 1. Checks that the protocol retries sending a command when an ACK is not 
 *  received within the timeout period.
 * 2. Validates that the protocol stops retrying after reaching the maximum
 *  number of retries.
 */
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

/**
 * Tests that the protocol sends the same frame on retry
 * 
 * Assertions:
 * 1. Checks that the same frame is sent on each retry attempt when an ACK is 
 *  not received.
 */
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

/** 
 * Tests that the protocol stops retrying when an ACK is received
 * 
 * Assertions:
 * 1. Checks that the protocol stops retrying when an ACK is received for a
 *  pending transaction.
 */
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

/** 
 * Tests that the protocol triggers a retry when a NACK is received
 * 
 * Assertions:
 * 1. Checks that the protocol retries sending a command when a NACK is received.
 */
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