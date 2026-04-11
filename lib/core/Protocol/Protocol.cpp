#include "Protocol.h"

void Protocol::set_write_callback(WriteCallback cb){
    this->write = cb;
}


void Protocol::send_frame(uint8_t seq, Command cmd, const uint8_t* payload, 
                          uint8_t len){
    if (!write) return;
    if (len > MAX_PAYLOAD) return;
    if (len > 0 && payload == nullptr) return;

    const uint8_t STX = 0x02;
    const uint8_t ETX = 0x03;

    write(STX);
    write(len);
    write(seq);
    write(static_cast<uint8_t>(cmd));

    for (uint8_t i = 0; i < len; i++){
        write(payload[i]);
    }

    uint8_t buffer[3 + MAX_PAYLOAD];
    buffer[0] = len;
    buffer[1] = seq;
    buffer[2] = static_cast<uint8_t>(cmd);
    
    for (uint8_t i = 0; i < len; i++){
        buffer[3 + i] = payload[i];
    }

    uint8_t crc = crc8(buffer, 3 + len);

    write(crc);

    write(ETX);
}


void Protocol::send_command(Command cmd, const uint8_t* payload, uint8_t len){
    if (pending_tx.waiting_ack) return;

    uint8_t seq = tx_seq++;

    this->send_frame(seq, cmd, payload, len);

    pending_tx.seq = seq;
    pending_tx.waiting_ack = true;
}


void Protocol::process(uint8_t byte){

    bool crc_ok = false;

    switch (this->parser.state) {
        
    case Parser::State::WAIT_STX:
        if (byte == 0x02) {
            this->parser.state = Parser::State::READ_LEN;
            this->parser.idx = 0;
        }
        break;

    case Parser::State::READ_LEN:
        this->parser.pkt.len = byte;

        if (this->parser.pkt.len > MAX_PAYLOAD) {
            this->parser.state = Parser::State::WAIT_STX;
        } else {
            this->parser.state = Parser::State::READ_SEQ;
        }
        break;

    case Parser::State::READ_SEQ:
        this->parser.pkt.seq = byte;
        this->parser.state = Parser::State::READ_CMD;
        break;

    case Parser::State::READ_CMD:
        this->parser.pkt.cmd = byte;

        if (this->parser.pkt.len == 0)
            this->parser.state = Parser::State::READ_CRC;
        else
            this->parser.state = Parser::State::READ_PAYLOAD;

        break;

    case Parser::State::READ_PAYLOAD:
        this->parser.pkt.payload[this->parser.idx++] = byte;

        if (this->parser.idx >= this->parser.pkt.len)
            this->parser.state = Parser::State::READ_CRC;

        break;

    case Parser::State::READ_CRC:
        this->parser.crc_received = byte;
        this->parser.state = Parser::State::WAIT_ETX;
        break;

    case Parser::State::WAIT_ETX:
        if (byte != 0x03) {
            this->reset_parser();
            return;
        }

        // valida CRC
        uint8_t temp[MAX_PAYLOAD + 3];
        temp[0] = this->parser.pkt.len;
        temp[1] = this->parser.pkt.seq;
        temp[2] = this->parser.pkt.cmd;

        for (uint8_t i = 0; i < this->parser.pkt.len; i++)
            temp[3 + i] = this->parser.pkt.payload[i];

        uint8_t crc = crc8(temp, this->parser.pkt.len + 3);
        crc_ok = (crc == this->parser.crc_received);

        auto pkt = this->parser.pkt;
        this->reset_parser();
        
        if (!crc_ok) {
            this->send_nack(pkt.seq, ErrorCode::INVALID_CRC);
            return;
        } 

        if (this->is_control_packet(pkt)){
            this->handle_control_packet(pkt);
            return;
        }

        this->send_ack(pkt.seq);
        this->queue_packet(pkt);
        
        return;
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
    if (!pending_tx.waiting_ack) return;

    if (pkt.seq != pending_tx.seq) return;

    if (pkt.cmd == (uint8_t)Command::ACK) {
        pending_tx.waiting_ack = false;
    }else if (pkt.cmd == (uint8_t)Command::NACK){
        pending_tx.waiting_ack = false;
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