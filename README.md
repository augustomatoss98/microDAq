📡 microDAQ

Continuous Monitoring & Data Acquisition Node (AVR / C++)

🚀 Overview

microDAQ is an embedded data acquisition and continuous monitoring system designed for AVR microcontrollers (ATmega328P).

It provides a robust and modular platform capable of:

Continuous sensor data acquisition
Reliable UART communication
Local data logging (SD card)
Remote command and monitoring

The system is built with a strong focus on:

Reliability
Modularity
Hardware abstraction
Testability outside the target hardware
🎯 Use Case

microDAQ acts as a continuous monitoring node, suitable for applications such as:

Industrial parameter monitoring
Environmental data logging
Equipment health tracking
Distributed sensing systems

The node continuously collects sensor data, logs it locally, and exposes it via a reliable UART protocol.

🏗️ Architecture
Application
   ↓
CommandHandler
   ↓
SensorManager
   ↓
Protocol
   ├── Parser (FSM)
   ├── Framing
   ├── CRC
   ├── ACK/NACK + Retry
   ↓
Core
   ├── SystemTime
   ↓
HAL
   ├── UART (ISR-driven)
   ├── SPI (SD Card)
   ├── Timer
Design Principles
Clear separation of concerns
Hardware-independent protocol layer
Deterministic behavior
High testability (runs on PC)
Scalable sensor integration
📡 Communication Protocol
Frame Format
[STX][LEN][SEQ][CMD][PAYLOAD][CRC][ETX]
Field	Description
STX	Start byte (0x02)
LEN	Payload length
SEQ	Sequence number
CMD	Command identifier
PAYLOAD	Data
CRC	CRC-8 checksum
ETX	End byte (0x03)
🔁 Reliability

The protocol implements a Stop-and-Wait ARQ mechanism:

ACK confirms successful reception
NACK signals errors (CRC, framing, etc.)
Automatic retransmission on timeout
Configurable retry limit (default: 2)

This ensures reliable communication even in noisy environments.

🧠 Parser (FSM)

Incoming data is processed byte-by-byte using a finite state machine:

WAIT_STX → READ_LEN → READ_SEQ → READ_CMD
→ READ_PAYLOAD → READ_CRC → WAIT_ETX
Features
Automatic resynchronization
CRC validation
Robust against corrupted streams
🌡️ Sensor System

Sensors are managed through a dedicated abstraction layer:

Common interface (ISensor)
Non-blocking update model
Scalable to multiple sensor types

The system supports continuous sampling and structured data retrieval via commands.

💾 Data Logging (SD Card)

microDAQ supports local data storage using an SD card:

Continuous logging of sensor data
Timestamped records
Configurable format (CSV or binary)
Buffered writes for efficiency
Fault handling for storage errors
🧪 Testing

The system is designed to be fully testable outside the embedded target.

Strategy
UART mocked via callbacks
Time abstraction (SystemTime) mocked
Frame capture and validation
Coverage
CRC validation
Parser FSM
Command handling
ACK/NACK behavior
Retry mechanism
Data integrity on retransmission
📦 Example
Request
CMD: READ_SENSOR (0x10)
PAYLOAD: [sensor_id]
Response
CMD: 0x90
PAYLOAD: [sensor_id][data...]
📊 System Flow
Sensor → SensorManager → CommandHandler → Protocol → UART
                                  ↓
                               SD Logger
📈 Roadmap
🔹 Communication
 Duplicate packet detection
 Extended error codes (NACK reasons)
 Flow control
 Sliding window
🔹 Sensors
 Implement ISensor interface
 SensorManager integration
 Real sensor drivers (temperature, ADC, etc.)
 Standardized payload format
🔹 Data Logging
 SD card integration (SPI)
 Continuous logging system
 File structure (CSV / binary)
 Buffered writes
 Write error handling
🔹 Telemetry & Logging
 System event logging
 Communication error logging
 Timestamping via SystemTime
 Log retrieval via UART
🔹 System
 Command set definition
 Log access commands
 Integration tests (Sensor + Protocol + Logging)
🧠 Key Differentials
Reliable UART protocol implemented from scratch
Fully testable without hardware
Clean layered architecture
Designed for continuous monitoring systems
Extensible to industrial-like applications
🎯 Project Goal

To evolve into a robust embedded data acquisition and monitoring platform, with:

Reliable communication
Scalable sensor support
Persistent data logging
Industrial-grade architecture
