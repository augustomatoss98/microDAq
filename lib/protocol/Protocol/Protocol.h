#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <cstdio>
#include "RingBuffer/RingBuffer.h"
#include "SystemTime/SystemTime.h"
#include "crc/crc.h"

class Protocol{
public:

    static constexpr uint8_t MAX_PAYLOAD = 32;
    static constexpr uint8_t ACK_TIMEOUT_MS = 50;
    static constexpr uint8_t MAX_RETRIES = 2;

    enum class Command : uint8_t {
        READ    = 0x01,
        WRITE   = 0x02,
        STATUS  = 0x03,
        PING    = 0x04,

        ACK     = 0xF0,
        NACK    = 0xF1
    };

    enum class ErrorCode : uint8_t{
        NONE            = 0x00,
        INVALID_CRC     = 0x01,
        INVALID_LENGTH  = 0x02,
        UNKNOWN_COMMAND = 0x03,
        INTERNAL_ERROR  = 0x04
    };

    struct Packet{
        uint8_t len;
        uint8_t seq;
        uint8_t cmd;
        uint8_t payload[MAX_PAYLOAD];
    };

    struct PendingTx {
        uint8_t seq;
        bool waiting_ack;

        uint8_t retry_count;
        uint32_t timestamp_ms;

        uint8_t buffer[MAX_PAYLOAD];
        uint8_t len;
        Command cmd;
    };

    PendingTx tx;

    using WriteCallback = void (*)(uint8_t);

    void process(uint8_t byte);
    bool available() const;
    Packet get_packet();
    virtual void send_command(Command cmd, const uint8_t* payload, 
                              uint8_t len);
    void set_write_callback(WriteCallback cb);
    void send_ack(uint8_t seq);
    void send_nack(uint8_t seq, ErrorCode err);
    void flush_rx_queue();
    void update();

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

        Packet pkt;

        uint8_t idx = 0;
        uint8_t crc_received = 0;
    };

    Parser parser;

    RingBuffer<Packet, 5> rx_queue;

    uint8_t tx_seq = 0;

    WriteCallback write = nullptr;

    void reset_parser();
    bool queue_packet(const Packet& pkt);
    void handle_control_packet(const Packet& pkt);

    void send_frame(uint8_t seq, Command cmd, const uint8_t* payload,
                    uint8_t len);

    bool is_control_packet(const Packet& pkt);
};

#endif