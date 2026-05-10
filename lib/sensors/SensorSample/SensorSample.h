#pragma once

#include <stdint.h>

struct SensorSample{
    uint32_t timestamp = 0;
    uint16_t value = 0;
};