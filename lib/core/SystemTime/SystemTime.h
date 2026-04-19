#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

#include <stdint.h>
#include "Utils/Atomic.h"

class SystemTime {
public:
    void init();
    static void tick();
    static uint32_t millis();
};

#endif