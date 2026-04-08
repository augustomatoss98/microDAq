#include "Protocol.h"

void Protocol::set_write_callback(WriteCallback cb){
    this->write = cb;
}


void Protocol::send(Command cmd, const uint8_t* payload, uint8_t len){
    if (!write) return;
    if (len > MAX_PAYLOAD) return;
    if (len == 0 && payload == nullptr) return;

    const uint8_t STX = 0x02;
    const uint8_t ETX = 0x03;

    uint8_t seq = sequence++;

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


void Protocol::process(uint8_t byte){

    switch (this->parser.state) {
        
    case Parser::State::WAIT_STX:
        if (byte == 0x02) {
            this->parser.state = Parser::State::READ_LEN;
            this->parser.idx = 0;
        }
        break;

    case Parser::State::READ_LEN:
        this->parser.len = byte;

        if (this->parser.len > MAX_PAYLOAD) {
            this->parser.state = Parser::State::WAIT_STX;
        } else {
            this->parser.state = Parser::State::READ_SEQ;
        }
        break;

    case Parser::State::READ_SEQ:
        this->parser.seq = byte;
        this->parser.state = Parser::State::READ_CMD;
        break;

    case Parser::State::READ_CMD:
        this->parser.cmd = byte;

        if (this->parser.len == 0)
            this->parser.state = Parser::State::READ_CRC;
        else
            this->parser.state = Parser::State::READ_PAYLOAD;

        break;

    case Parser::State::READ_PAYLOAD:
        this->parser.buffer[this->parser.idx++] = byte;

        if (this->parser.idx >= this->parser.len)
            this->parser.state = Parser::State::READ_CRC;

        break;

    case Parser::State::READ_CRC:
        this->parser.crc_received = byte;
        this->parser.state = Parser::State::WAIT_ETX;
        break;

    case Parser::State::WAIT_ETX:
        if (byte == 0x03) {

            // valida CRC
            uint8_t temp[64];
            temp[0] = this->parser.len;
            temp[1] = this->parser.seq;
            temp[2] = this->parser.cmd;

            for (uint8_t i = 0; i < this->parser.len; i++)
                temp[3 + i] = this->parser.buffer[i];

            uint8_t crc = crc8(temp, parser.len + 3);

            if (crc == this->parser.crc_received) {
                this->handle_packet();
            }
        }
    }
}


void Protocol::handle_packet(){
    this->current_packet.len = this->parser.len;
    this->current_packet.cmd = this->parser.cmd;
    this->current_packet.seq = this->parser.seq;

    for (uint8_t i = 0; i < this->parser.len; i++){
        this->current_packet.payload[i] = this->parser.buffer[i];
    }
    
    this->packet_ready = true;
    this->reset_parser();
}


void Protocol::reset_parser(){
    this->parser.state = Parser::State::WAIT_STX;
    this->parser.idx = 0;
    this->parser.len = 0;
    this->parser.seq = 0;
    this->parser.cmd = 0;
    this->parser.crc_received = 0;
}


Protocol::Packet Protocol::get_packet(){
    this->packet_ready = false;
    return this->current_packet;
}


bool Protocol::available() const{
    return this->packet_ready;
}

