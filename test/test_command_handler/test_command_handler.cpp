#include <unity.h>
#include "CommandHandler/CommandHandler.h"
#include "Protocol/Protocol.h"
#include "SensorManager/SensorManager.h"

class MockProtocol : public Protocol {
public:

    Command last_cmd;
    uint8_t last_payload[32];
    uint8_t last_len = 0;
    bool called = false;

    void send_command(Command cmd, const uint8_t* payload, uint8_t len) override {
        called = true;
        last_cmd = cmd;
        last_len = len;

    for (uint8_t i = 0; i < len; i++) {
        last_payload[i] = payload[i];
        };
    };
};

class FakeSensorManager : public SensorManager {
public:
    uint16_t read(uint8_t sensor_id) override {
        return 0x1234 + sensor_id;
    };
};

FakeSensorManager sensor;
CommandHandler handler(sensor);
MockProtocol protocol;

void setUp(){
    protocol.called = false;
};

void tearDown(){};

void test_read_command(){
    Protocol::Packet pkt;
    pkt.cmd = (uint8_t)Protocol::Command::READ;
    pkt.len = 1;
    pkt.payload[0] = 2;

    handler.handle(pkt, protocol);

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

void test_ping_command(){
    Protocol::Packet pkt;
    pkt.cmd = (uint8_t)Protocol::Command::PING;
    pkt.len = 0;

    handler.handle(pkt, protocol);

    TEST_ASSERT_TRUE(protocol.called);
    TEST_ASSERT_EQUAL_UINT8(
        (uint8_t)Protocol::Command::PING,
        (uint8_t)protocol.last_cmd
    );

    TEST_ASSERT_EQUAL_UINT8(0, protocol.last_len);
}

void test_read_invalid_len() {
    Protocol::Packet pkt;
    pkt.cmd = (uint8_t)Protocol::Command::READ;
    pkt.len = 0; // inválido

    handler.handle(pkt, protocol);

    TEST_ASSERT_FALSE(protocol.called);
}

int main(){
    UNITY_BEGIN();
    RUN_TEST(test_read_command);
    RUN_TEST(test_ping_command);
    RUN_TEST(test_read_invalid_len);
    return UNITY_END();
}