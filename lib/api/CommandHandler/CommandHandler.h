#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "Protocol/Protocol.h"
#include "SensorManager/SensorManager.h"

/**
 * @brief CommandHandler class for processing protocol commands
 * 
 * This class is responsible for handling incoming protocol packets,
 * and executing the appropriate actions based on the command type. It
 * interfaces with the SensorManager to perform sensor read/write operations
 * and generates responses accordingly.
 * 
 * Supported commands:
 * - PING: Responds with a PING to acknowledge connectivity.
 * - READ: Reads a sensor value based on the provided sensor ID and responds
 *  with the value.
 * - STATUS: Responds with a status byte (currently always OK).
 * - WRITE: Placeholder for future implementation of sensor write operations.
 */
class CommandHandler {
public:
    /**
     * @brief Construct a new Command Handler object 
     * @param sensor Reference to the SensorManager for handling sensor 
     * operations
     */
    CommandHandler(SensorManager& sensor);

    /**
     * @brief Handle an incoming protocol packet
     * 
     * Processes the incoming packet based on its command type, interecting 
     * with the corresponding classes (e.g., SensorManager) to execute the 
     * command and generate a response.
     * 
     * @param pkt The incoming protocol packet to be handled
     * @param protocol Reference to the Protocol instance for sending responses
     */
    void handle(const Protocol::Packet& pkt, Protocol& protocol);

private:
    SensorManager& sensor;
};

#endif