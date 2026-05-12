#pragma once

#include <stdint.h>
#include <Utils/Atomic.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "RingBuffer/RingBuffer.h" 

/** @brief A class for handling UART communication
 * 
 * This class provides an interface for initializing and using the UART 
 * peripheral on AVR microcontrollers. It sets a non-blocking communication
 * using ring buffers and interrupts for both transmission and reception.
 * 
 * The class implements methods for writing data to the UART, checking for 
 * available data, reading received data, and flushing the buffers.
 */
class UART{
public:
    /** 
     * @brief Initializes the UART with the specified baud rate
     * 
     * This function sets up the UART peripheral with the given baud rate and 
     * enables the necessary pins for transmission and reception. 
     * 
     * @param baud The desired baud rate for UART communication
     */
    static void init(uint32_t baud);

    /** 
     * @brief Writes data into the UART
     * 
     * This function adds the specified data to the transmit buffer and enables
     * the transmit interrupt if the buffer is not full. The data transmission
     * is handled assynchronously through interrupts, alowing the main program
     *  to continue executing without blocking.
     * 
     * @param data The data to be written to the UART
     * @return true if the data was successfully written, false otherwise
     */
    static bool write(uint8_t data);

    /** 
     * @brief Writes multiple bytes into the UART
     * 
     * This function adds the specified data to the transmit buffer and 
     * enables the transmit interrupt if the buffer is not full.
     * 
     * @param data A pointer to the data to be written to the UART
     * @param len The number of bytes to write
     * @return true if the data was successfully written, false 
     * otherwise
     */
    static bool write_bytes(const uint8_t* data, size_t len);

    /** 
     * @brief Checks if data is available in the receive buffer
     * 
     * This function checks if there is data available in the 
     * receive buffer.
     * 
     * @return true if data is available, false otherwise
     */
    static bool is_available();

    /** 
     * @brief Reads data from the UART
     * 
     * This function removes data from the receive buffer and returns it.
     * The data reception is handled asynchronously through interrupts, allowing
     * the main program to continue executing without blocking. 
     * 
     * @return The data read from the UART
     */
    static uint8_t read();

    /** 
     * @brief Flushes the transmit and receive buffers
     * 
     * This function clears all data from both the transmit and receive buffers.
     */
    static void flush();

    // Ring buffers for TX and RX
    static RingBuffer<uint8_t, 64> tx_buffer;
    static RingBuffer<uint8_t, 64> rx_buffer;

private:
    /** 
     * @brief Sets the baud rate for the UART
     * 
     * This function calculates the appropriate value for the baud rate register
     * based on the desired baud rate and the system clock frequency.
     * 
     * @param baud The desired baud rate for UART communication
     */
    static void set_baudrate(uint32_t baud);

    /** 
     * @brief Enables the transmit interrupt for the UART
     * 
     * This function enables the transmit interrupt, which will be triggered when
     * the transmit buffer is empty and data can be sent.
     */
    static void enable_tx_interrupt();
};
