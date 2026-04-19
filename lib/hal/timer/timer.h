#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "SystemTime/SystemTime.h"



namespace Timer{
    void timer0_init();

    constexpr uint16_t F_TIMER0 = 1000UL;
    constexpr uint8_t SCALER = 64UL;
};



#endif