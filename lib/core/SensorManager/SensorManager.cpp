#include "SensorManager.h"

uint16_t SensorManager::read(uint8_t sensor_id) {
    return 1000 + sensor_id;
}