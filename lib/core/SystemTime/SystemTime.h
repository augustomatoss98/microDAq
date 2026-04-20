#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <stdint.h>

#include "Utils/Atomic.h"


/** 
 * @brief System time management class
 * 
 *  This class provides functionality for managing system time, including
 *  initialization, tick updates, and millisecond retrieval.
 */
class SystemTime {
public:
    /** @brief Initialize the system time */
    void init();

    /** @brief Update the system time tick. Is called from ISR */
    static void tick();

    /** @brief Get the current time in milliseconds */
    static uint32_t millis();
};

#endif