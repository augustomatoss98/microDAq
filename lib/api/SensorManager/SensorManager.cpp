#include "SensorManager.h"

SensorManager::SensorManager() : sensor_count(0){
    //NOP
}

bool SensorManager::add_sensor(uint8_t id, ISensor& sensor){
    if(sensor_count >= MAX_SENSORS) return false;

    sensors[sensor_count].id = id;
    sensors[sensor_count].sensor = &sensor;
    sensor_count++;

    return true;
}

void SensorManager::process(){
    for(uint8_t i = 0; i < sensor_count; i++){
        sensors[i].sensor->update();
    }
}



bool SensorManager::read(uint8_t sensor_id, SensorSample& sample) {
    for(uint8_t i = 0; i < sensor_count; i++){
        if (sensors[i].id == sensor_id){
            sample = sensors[i].sensor->get_sample();
            return true;
        }
    }
    return false;
}

uint8_t SensorManager::get_sensor_count(){
    return this->sensor_count;
}