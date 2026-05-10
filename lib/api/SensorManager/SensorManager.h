#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stdint.h>

#include "SensorSample/SensorSample.h"
#include "ISensor/ISensor.h"

class SensorManager{
public:
    static constexpr uint8_t MAX_SENSORS = 8;
    
    struct SensorEntry{
        uint8_t id = 0;
        ISensor* sensor = nullptr;
    };

    SensorManager();

    bool add_sensor(uint8_t id, ISensor& sensor);
    
    virtual bool read(uint8_t sensor_id, SensorSample& sample);

    void process();

    uint8_t get_sensor_count();

private:
    SensorEntry sensors[MAX_SENSORS];

    uint8_t sensor_count;
};


#endif