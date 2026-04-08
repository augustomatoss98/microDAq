#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "crc/crc.h"

#define MAX_PAYLOAD 32

class Protocol{
public:
    enum class Command : uint8_t {
        READ    = 0x01,
        WRITE   = 0x02,
        STATUS  = 0x03,
        PING    = 0x04
    };

    struct Packet{
        uint8_t cmd;
        uint8_t len;
        uint8_t payload[MAX_PAYLOAD];
        uint8_t seq;
    };

    using WriteCallback = void (*)(uint8_t);

    void process(uint8_t byte);
    bool available() const;
    Packet get_packet();
    virtual void send(Command cmd, const uint8_t* payload, uint8_t len);
    void set_write_callback(WriteCallback cb);

private:
    struct Parser{

        enum class State {
            WAIT_STX,
            READ_LEN,
            READ_SEQ,
            READ_CMD,
            READ_PAYLOAD,
            READ_CRC,
            WAIT_ETX
        };

        State state = State::WAIT_STX;

        uint8_t buffer[MAX_PAYLOAD];
        uint8_t idx = 0;

        uint8_t len = 0;
        uint8_t seq = 0;
        uint8_t cmd = 0;

        uint8_t crc_received = 0;
    };

    Parser parser;

    Packet current_packet;
    bool packet_ready = false;

    uint8_t sequence = 0;

    WriteCallback write = nullptr;

    void reset_parser();
    void handle_packet();
};

#endif