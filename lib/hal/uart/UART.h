#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <util/atomic.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "RingBuffer.h" 

#define F_CPU 16000000UL

class UART{
public:
    static void init(uint32_t baud);
    
    static bool write(uint8_t data);
    static bool write(const uint8_t* data, size_t len);

    static bool is_available();
    static uint8_t read();

    static void flush();

    static RingBuffer<uint8_t, 64> tx_buffer;
    static RingBuffer<uint8_t, 64> rx_buffer;

private:
    static void set_baudrate(uint32_t baud);
    static void enable_tx_interrupt();
};

#endif