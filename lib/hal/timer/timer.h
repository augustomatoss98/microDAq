#pragma once

#include <stdint.h>

#include "SystemTime/SystemTime.h"


#ifdef __AVR__

#include <avr/io.h>
#include <avr/interrupt.h>


/**
 * @brief Timer management for system timekeeping
 * 
 * This namespace provides functionality for initializing and managing the
 * hardware timers used for system timekeeping. It includes functionalities 
 * to initialize Timer0 for generating periodic interrupts to update the system
 * time.
 */
namespace Timer{
    /** @brief Initialize Timer0 for system timekeeping */
    void timer0_init();

    constexpr uint16_t F_TIMER0 = 1000UL;
    constexpr uint8_t SCALER = 64UL;
};

#else

namespace Timer{
    /** @brief Initialize Timer0 for system timekeeping */
    void timer0_init();

    constexpr uint16_t F_TIMER0 = 1000UL;
    constexpr uint8_t SCALER = 64UL;
};

#endif