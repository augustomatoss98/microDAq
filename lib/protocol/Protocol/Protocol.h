#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#include "RingBuffer/RingBuffer.h"
#include "SystemTime/SystemTime.h"
#include "crc/crc.h"


/** @brief Protocol class for handling communication protocol
 * 
 * This class implements a simple communication protocol with the following 
 * features:
 * 
 * - Packet structure with STX, LEN, SEQ, CMD, PAYLOAD, CRC, ETX
 * - Command types: READ, WRITE, STATUS, PING, ACK, NACK
 * - Error handling with NACK and error codes
 * - Retries for failed transmissions
 * - Ring buffer for incoming packets
 * - Independent of transport layer (uses callback for sending bytes)
 * 
 * Designed for limted resources and simplicity, suitable for microcontroller communication.
 */
class Protocol{
public:

    // Class constants
    static constexpr uint8_t MAX_PAYLOAD = 32;
    static constexpr uint8_t ACK_TIMEOUT_MS = 50;
    static constexpr uint8_t MAX_RETRIES = 2;

    // Protocol commands
    enum class Command : uint8_t {
        READ    = 0x01,
        WRITE   = 0x02,
        STATUS  = 0x03,
        PING    = 0x04,

        ACK     = 0xF0,
        NACK    = 0xF1
    };

    // Error codes for NACK
    enum class ErrorCode : uint8_t{
        NONE            = 0x00,
        INVALID_CRC     = 0x01,
        INVALID_LENGTH  = 0x02,
        UNKNOWN_COMMAND = 0x03,
        INTERNAL_ERROR  = 0x04
    };

    // Packet structure
    struct Packet{
        uint8_t len;
        uint8_t seq;
        uint8_t cmd;
        uint8_t payload[MAX_PAYLOAD];
    };

    // Pending transmission structure
    struct PendingTx {
        uint8_t seq = 0;
        bool waiting_ack = false;

        uint8_t retry_count;
        uint32_t timestamp_ms;

        uint8_t buffer[MAX_PAYLOAD];
        uint8_t len;
        Command cmd;
    };

    // Write callback type
    using WriteCallback = bool (*)(uint8_t);

    /** 
     * @brief Process incoming byte
     * 
     * This function processes each incoming byte and updates the parser state accordingly.
     * 
     * @param byte The incoming byte to process
     */
    void process(uint8_t byte);

    /** 
     * @brief Check if there are available packets
     * 
     * @return true if there are available packets, false otherwise
     */
    bool available() const;

    /** 
     * @brief Get the next available packet
     * 
     * @return The next available packet
     */
    Packet get_packet();

    /** 
     * @brief Flush the receive queue
     * 
     * This function clears all packets from the receive queue.
     */
    void flush_rx_queue();

    /** 
     * @brief Send a command via the transport layer
     * 
     * @param cmd The command to send
     * @param payload The payload to send
     * @param len The length of the payload
     */
    virtual void send_command(Command cmd, const uint8_t* payload, 
                              uint8_t len);

    /** 
     * @brief Set the write callback function for sending bytes
     * 
     * @param cb The write callback to be set
     */
    void set_write_callback(WriteCallback cb);

    /** 
     * @brief Send an acknowledgment packet
     * 
     * @param seq The sequence number of the acknowledgment
     */
    void send_ack(uint8_t seq);

    /** 
     * @brief Send a negative acknowledgment packet
     * 
     * @param seq The sequence number of the negative acknowledgment
     * @param err The error code for the negative acknowledgment
     */
    void send_nack(uint8_t seq, ErrorCode err);

    /** 
     * @brief Update the protocol state
     * 
     * This function should be called periodically to handle timeouts and 
     * other time-sensitive operations.
     */
    void update();
    
    PendingTx tx;

private:
    // Parser state machine for incoming bytes
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

    /** 
     * @brief Reset the parser state
     */
    void reset_parser();

    /** 
     * @brief Queue a packet for processing
     * 
     * @param pkt The packet to queue
     * @return true if the packet was successfully queued, false otherwise
     */
    bool queue_packet(const Packet& pkt);

    /** 
     * @brief Handle a control packet
     * 
     * @param pkt The control packet to handle
     */
    void handle_control_packet(const Packet& pkt);

    /** 
     * @brief Send a data frame via the transport layer
     * 
     * @param seq The sequence number of the frame
     * @param cmd The command for the frame
     * @param payload The payload for the frame
     * @param len The length of the payload
     */
    void send_frame(uint8_t seq, Command cmd, const uint8_t* payload,
                    uint8_t len);
    
    /** 
     * @brief Check if a packet is a control packet
     * 
     * @param pkt The packet to check
     * @return true if the packet is a control packet, false otherwise
     */
    bool is_control_packet(const Packet& pkt);

    Parser parser;

    RingBuffer<Packet, 5> rx_queue;

    uint8_t tx_seq = 0;

    WriteCallback write = nullptr;
};

#endif