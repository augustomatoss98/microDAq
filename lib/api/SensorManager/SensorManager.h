#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stdint.h>

class SensorManager{
public:
    virtual uint16_t read(uint8_t sensor_id);
};


#endif