#include "UART.h"

RingBuffer<uint8_t, 64> UART::tx_buffer;
RingBuffer<uint8_t, 64> UART::rx_buffer;

void UART::init(uint32_t baud){    
    UART::set_baudrate(baud);

    /* Enable receiver and transmitter pins*/
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);

    /* Set frame format: 8-bit data, 1 stop-bit, no parity*/
    UCSR0C = (3<<UCSZ00);

    /*Enables interrupts*/
    sei();
}

bool UART::write(uint8_t data){
    bool ret;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        ret = UART::tx_buffer.push(data);
    }

    if(!ret) return false;

    UART::enable_tx_interrupt();
    return true;
}

bool UART::write(const uint8_t* data, size_t len){
    for(size_t i = 0; i < len; i++){
        if(!UART::write(data[i])) return false;
    }

    return true;
}

uint8_t UART::read(){
    uint8_t data = 0;
    UART::rx_buffer.pop(data);
    
    return data;
}


void UART::flush(){
    UART::rx_buffer.flush();
    UART::tx_buffer.flush();
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


void UART::enable_tx_interrupt(){
    UCSR0B |= (1 << UDRIE0);
}

bool UART::is_available(){
    return !UART::rx_buffer.is_empty();
}

ISR(USART_UDRE_vect){
    uint8_t data;

    if(!UART::tx_buffer.pop(data)){
        UCSR0B &= ~(1 << UDRIE0);
        return;
    }

    UDR0 = data;
}


ISR(USART_RX_vect){
    uint8_t data = UDR0;
    UART::rx_buffer.push(data);
}

