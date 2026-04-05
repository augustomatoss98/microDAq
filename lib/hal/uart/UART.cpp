#include "uart.h"

void UART::init(uint32_t baud){

    uint16_t ubrr;
    
    UART::set_baudrate(baud);

    /* Enable receiver and transmitter pins*/
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);

    /* Set frame format: 8-bit data, 1 stop-bit, no parity*/
    UCSR0C = (3<<UCSZ00);
}

void UART::write(unsigned char data){
    while(!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

void UART::write(const char* str){
    while(*str){
        UART::write(*str++);
    }
}

unsigned char UART::read(void){
    while(!(UCSR0A & (1<<RXC0)));

    return UDR0;
}

void UART::set_baudrate(uint32_t baud){
    uint16_t ubrr;
    
    /*For baud values bigger than 57600 the UART may malfuncition 
    if not in fast mode*/
    if (baud >= 57600){
        /*Sets the UART port to fast mode*/
        UCSR0A = UCSR0A | (1<<U2X0);
        /*Calculates the register value for desired baud rate*/
        ubrr = (F_CPU/(8UL*baud)) - 1;
    }else{
        /*Calculates the register value for desired baud rate*/
        ubrr = (F_CPU/(16UL*baud)) - 1;
    }

    /*Sets the baudrate in the Baudrate register*/
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;

}