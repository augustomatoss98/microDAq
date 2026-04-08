#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "Protocol/Protocol.h"
#include "SensorManager/SensorManager.h"

class CommandHandler {
public:
    CommandHandler(SensorManager& sensor);

    void handle(const Protocol::Packet& pkt, Protocol& protocol);

private:
    SensorManager& sensor;
};

#endif