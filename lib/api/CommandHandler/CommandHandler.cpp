#include "CommandHandler.h"
#include <cstdio>

CommandHandler::CommandHandler(SensorManager& sensor) : sensor(sensor){};

void CommandHandler::handle(const Protocol::Packet& pkt, Protocol& protocol) {
    switch (pkt.cmd) {

    case (int)Protocol::Command::PING: {
        protocol.send_command(Protocol::Command::PING, nullptr, 0);
    }

    case (int)Protocol::Command::READ: {
        SensorSample sample;
        uint8_t sensor_id;
        uint8_t resp[3*pkt.len];

        printf("%d\n", pkt.len);
        printf("%d\n", sensor.MAX_SENSORS);

        if ((pkt.len < 1) || (pkt.len > sensor.MAX_SENSORS)) return; // validação básica

        for(uint8_t i = 0; i < pkt.len; i++){
            sensor_id = pkt.payload[i];
            this->sensor.read(sensor_id, sample);
            
            resp[3*i] = sensor_id;
            resp[3*i+1] = (sample.value >> 8) & 0xFF;
            resp[3*i+2] = sample.value & 0xFF;
        }

        protocol.send_command(Protocol::Command::READ, resp, 3*pkt.len);
        break;
    }

    case (int)Protocol::Command::STATUS: {

        uint8_t status = 0x00; // OK

        protocol.send_command(Protocol::Command::STATUS, &status, 1);
        break;
    }

    case (int)Protocol::Command::WRITE: {
        // ainda não implementado
        break;
    }

    default:
        // ignora por enquanto
        break;
    }
}