#include "SystemTime.h"

namespace {
    volatile uint64_t time_ms = 0;
}

void SystemTime::init(){
    time_ms = 0;
}

void SystemTime::tick(){
    time_ms++;
}

uint32_t SystemTime::millis(){
    uint32_t t;
    ATOMIC_SECTION {
        t = time_ms;
    }
    return t;
}