#include <unity.h>
#include "CommandHandler/CommandHandler.h"
#include "Protocol/Protocol.h"
#include "SensorManager/SensorManager.h"

/**
 * @brief Mock implementation of Protocol for testing CommandHandler
 * 
 * This mock class captures the last command sent and its payload, allowing
 * tests to verify that CommandHandler generates the correct responses based
 * on the incoming packets. It overrides the send_command method to store
 * the command and payload for later verification in tests.
 */
class MockProtocol : public Protocol {
public:

    Command last_cmd;
    uint8_t last_payload[32];
    uint8_t last_len = 0;
    bool called = false;

    void send_command(Command cmd, const uint8_t* payload, uint8_t len) 
        override {
        called = true;
        last_cmd = cmd;
        last_len = len;

    for (uint8_t i = 0; i < len; i++) {
        last_payload[i] = payload[i];
        };
    };
};

/**
 * @brief Fake SensorManager for testing CommandHandler
 * 
 * This implementation of SensorManager generates predictable sensor
 * readings allowing the READ command to be tested without actual hardware.
 */
class FakeSensorManager : public SensorManager {
public:
    bool read(uint8_t sensor_id, SensorSample& sample) override {
        sample.value = 0x1234 + sensor_id;
        return true;
    };
};

FakeSensorManager sensor;
CommandHandler handler(sensor);
MockProtocol protocol;

// Reset state before each test
void setUp(){
    protocol.called = false;
};

// Clean up after each test
void tearDown(){};

/**
 * Tests the READ command handling of CommandHandler
 * 
 * Assertions:
 * 1. Checks if a response was generated.
 * 2. Verifies that the response command is correct (READ).
 * 3. Validates that the payload length is correct (3 bytes).
 * 4. Confirms that the sensor ID in the payload matches the request.
 * 5. Validates that the sensor value in the payload is correct,
 * covering multi-byte values.
 */
void test_read_command(){
    // Generates test packet
    SensorSample sample;
    Protocol::Packet pkt;
    pkt.cmd = (uint8_t)Protocol::Command::READ;
    pkt.len = 1;
    pkt.payload[0] = 2;

    // Calls the handler with the test packet
    handler.handle(pkt, protocol);

    // Assertions
    TEST_ASSERT_TRUE(protocol.called);
    TEST_ASSERT_EQUAL_UINT8(
        (uint8_t)Protocol::Command::READ,
        (uint8_t)protocol.last_cmd
    );

    TEST_ASSERT_EQUAL_UINT8(3, protocol.last_len);

    TEST_ASSERT_EQUAL_UINT8(2, protocol.last_payload[0]);

    uint16_t expected = 0x1234 + 2;

    TEST_ASSERT_EQUAL_UINT8(expected >> 8, protocol.last_payload[1]);
    TEST_ASSERT_EQUAL_UINT8(expected & 0xFF, protocol.last_payload[2]);
}

/**
 * Test handling of PING command
 * 
 * Assertions:
 * 1. Checks if a response was generated.
 * 2. Verifies that the response command is correct (PING).
 * 3. Validates that the payload length is correct (0 bytes).
 */
void test_ping_command(){
    // Creates test packet
    Protocol::Packet pkt;
    pkt.cmd = (uint8_t)Protocol::Command::PING;
    pkt.len = 0;

    // Calls the handler with the test packet
    handler.handle(pkt, protocol);

    // Assertions
    TEST_ASSERT_TRUE(protocol.called);
    TEST_ASSERT_EQUAL_UINT8(
        (uint8_t)Protocol::Command::PING,
        (uint8_t)protocol.last_cmd
    );

    TEST_ASSERT_EQUAL_UINT8(0, protocol.last_len);
}

/**
 * Test handling of READ command with invalid length
 * 
 * 1. Checks that no response is generated when the READ command has an 
 * invalid length (0 bytes).
 */
void test_read_invalid_len() {
    // Creates test packet with invalid length
    Protocol::Packet pkt;
    pkt.cmd = (uint8_t)Protocol::Command::READ;
    pkt.len = 0;

    // Calls the handler with the test packet
    handler.handle(pkt, protocol);

    // Assertions
    TEST_ASSERT_FALSE(protocol.called);
}

int main(){
    UNITY_BEGIN();

    RUN_TEST(test_read_command);
    RUN_TEST(test_ping_command);
    RUN_TEST(test_read_invalid_len);

    return UNITY_END();
}