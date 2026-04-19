#include "Protocol.h"

void Protocol::set_write_callback(WriteCallback cb){
    this->write = cb;
}


void Protocol::send_frame(uint8_t seq, Command cmd, const uint8_t* payload, 
                          uint8_t len){
    if (!write) return;                         // no callback set
    if (len > MAX_PAYLOAD) return;              // payload too large
    if (len > 0 && payload == nullptr) return;  // payload pointer invalid

    const uint8_t STX = 0x02;
    const uint8_t ETX = 0x03;

    // Write frame header | Frame: [STX][LEN][SEQ][CMD][PAYLOAD][CRC][ETX]
    write(STX);
    write(len);
    write(seq);
    write(static_cast<uint8_t>(cmd));
    
    // Write payload
    for (uint8_t i = 0; i < len; i++) write(payload[i]);


    // Calculate CRC over LEN, SEQ, CMD and PAYLOAD
    uint8_t buffer[3 + MAX_PAYLOAD];

    buffer[0] = len;
    buffer[1] = seq;
    buffer[2] = static_cast<uint8_t>(cmd);

    for (uint8_t i = 0; i < len; i++) buffer[3 + i] = payload[i];
    uint8_t crc = crc8(buffer, 3 + len);


    // Write CRC and ETX
    write(crc);
    write(ETX);
}


void Protocol::send_command(Command cmd, const uint8_t* payload, uint8_t len){
    if (this->tx.waiting_ack) return;   // Blocks if waiting for ACK
    uint8_t seq = tx_seq++;             // Increment sequence number          

    // Send the frame
    this->send_frame(seq, cmd, payload, len);

    // Store the pending transaction for potential retries
    this->tx.seq = seq;
    this->tx.cmd = cmd;
    this->tx.len = len;
    for (size_t i = 0; i < len; i++){
        this->tx.buffer[i] = payload[i];
    }
    
    // Initialize retry state
    this->tx.retry_count = 0;
    this->tx.waiting_ack = true;
    this->tx.timestamp_ms = SystemTime::millis();
}


void Protocol::process(uint8_t byte){
    bool crc_ok = false;    // CRC validation flag

    // State machine to parse incoming bytes into packets
    switch (this->parser.state) {
    
    // STX byte - looks for STX in data stream
    case Parser::State::WAIT_STX:
        if (byte == 0x02) {
            this->parser.state = Parser::State::READ_LEN;
            this->parser.idx = 0;
        }
        break;
    
    // Length byte - validates payload length
    case Parser::State::READ_LEN:
        this->parser.pkt.len = byte;

        if (this->parser.pkt.len > MAX_PAYLOAD) {
            // Invalid length, reset parser
            this->parser.state = Parser::State::WAIT_STX;
        } else {
            this->parser.state = Parser::State::READ_SEQ;
        }
        break;
    // Sequence byte - captures packet sequence number
    case Parser::State::READ_SEQ:
        this->parser.pkt.seq = byte;
        this->parser.state = Parser::State::READ_CMD;
        break;

    // Command byte - captures command and determines next state
    case Parser::State::READ_CMD:
        this->parser.pkt.cmd = byte;

        if (this->parser.pkt.len == 0)
            this->parser.state = Parser::State::READ_CRC;
        else
            this->parser.state = Parser::State::READ_PAYLOAD;

        break;
    
    // Payload bytes - reads payload data based on length
    case Parser::State::READ_PAYLOAD:
        this->parser.pkt.payload[this->parser.idx++] = byte;

        if (this->parser.idx >= this->parser.pkt.len)
            this->parser.state = Parser::State::READ_CRC;

        break;
    
    // CRC byte - captures CRC for validation
    case Parser::State::READ_CRC:
        this->parser.crc_received = byte;
        this->parser.state = Parser::State::WAIT_ETX;
        break;
    
    // ETX byte - validates end of frame
    case Parser::State::WAIT_ETX:
        if (byte != 0x03) {
            this->reset_parser();
            return;
        }

        // CRC Validation - calculates CRC and compares with received value
        uint8_t temp[MAX_PAYLOAD + 3];
        temp[0] = this->parser.pkt.len;
        temp[1] = this->parser.pkt.seq;
        temp[2] = this->parser.pkt.cmd;

        for (uint8_t i = 0; i < this->parser.pkt.len; i++)
            temp[3 + i] = this->parser.pkt.payload[i];

        uint8_t crc = crc8(temp, this->parser.pkt.len + 3);
        crc_ok = (crc == this->parser.crc_received);

        // Store the packet before resetting the parser to ensure data integrity
        auto pkt = this->parser.pkt;
        this->reset_parser();
        
        // Handle packet based on CRC validation and control packet status
        if (!crc_ok) {
            this->send_nack(pkt.seq, ErrorCode::INVALID_CRC);
            return;
        } 

        if (this->is_control_packet(pkt)){
            this->handle_control_packet(pkt);
            return;
        }

        // Valid packet received - send ACK and queue for processing
        this->send_ack(pkt.seq);
        this->queue_packet(pkt);
        
        return;
    }
}


void Protocol::update(){
    if (!this->tx.waiting_ack) return;  // No pending transaction, nothing to do

    // Check if ACK timeout has occurred
    if (SystemTime::millis() - this->tx.timestamp_ms >= 
            Protocol::ACK_TIMEOUT_MS){
        
        // Timeout occurred, check if we can retry
        if (this->tx.retry_count < Protocol::MAX_RETRIES){
            this->send_frame(this->tx.seq, this->tx.cmd,
                             this->tx.buffer, this->tx.len);

            this->tx.retry_count++;
            this->tx.timestamp_ms = SystemTime::millis();
        }else{
            this->tx.waiting_ack = false;
        }
    }
}


bool Protocol::queue_packet(const Packet& p){
    if (this->rx_queue.is_full()) {
        return false;
    }

    this->rx_queue.push(p);
    return true;
}


void Protocol::handle_control_packet(const Packet& pkt){
    // No pending transaction, ignore control packets
    if (!this->tx.waiting_ack) return;

    // Control packet does not match pending transaction, ignore
    if (pkt.seq != this->tx.seq) return;

    // Handle ACK/NACK for the pending transaction
    if (pkt.cmd == (uint8_t)Command::ACK) {
        this->tx.waiting_ack = false;
    }else if (pkt.cmd == (uint8_t)Command::NACK){
        this->tx.timestamp_ms = 0; // Force immediate retry on next update
    }
}


void Protocol::reset_parser(){
    this->parser.state = Parser::State::WAIT_STX;
    this->parser.pkt = {};
    this->parser.idx = 0;
    this->parser.crc_received = 0;
}


void Protocol::send_ack(uint8_t seq){
    send_frame(seq, Command::ACK, nullptr, 0);
}


void Protocol::send_nack(uint8_t seq, ErrorCode err) {
    uint8_t payload[1] = {static_cast<uint8_t>(err)};
    send_frame(seq, Command::NACK, payload, 1);
}


Protocol::Packet Protocol::get_packet(){
    Packet pkt;

    if (this->rx_queue.pop(pkt)){
        return pkt;
    };
    
    return {};
}


bool Protocol::available() const{
    return !this->rx_queue.is_empty();
}


bool Protocol::is_control_packet(const Packet& pkt){
    return (pkt.cmd == (uint8_t)Command::ACK || pkt.cmd == (uint8_t)Command::NACK);
}


void Protocol::flush_rx_queue(){
    this->rx_queue.flush();
}