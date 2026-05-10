#include "CommandHandler.h"

CommandHandler::CommandHandler(SensorManager& sensor) : sensor(sensor){};

void CommandHandler::handle(const Protocol::Packet& pkt, Protocol& protocol) {
    switch (pkt.cmd) {

    case (int)Protocol::Command::PING: {
        protocol.send_command(Protocol::Command::PING, nullptr, 0);
    }

    case (int)Protocol::Command::READ: {
        SensorSample sample;

        if (pkt.len < 1) return; // validação básica

        uint8_t sensor_id = pkt.payload[0];
        sensor.read(sensor_id, sample);

        uint8_t resp[3];
        resp[0] = sensor_id;
        resp[1] = (sample.value >> 8) & 0xFF;
        resp[2] = sample.value & 0xFF;

        protocol.send_command(Protocol::Command::READ, resp, 3);
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