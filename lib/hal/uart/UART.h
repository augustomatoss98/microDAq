#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <avr/io.h>

#define F_CPU 16000000UL

class UART{
public:
    static void init(uint32_t baud);
    
    static void write(unsigned char data);
    static void write(const char* str);

    static unsigned char read(void);


private:
    static void set_baudrate(uint32_t baud);
};

#endif