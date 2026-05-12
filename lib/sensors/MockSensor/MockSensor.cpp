#include "MockSensor.h"

MockSensor::MockSensor(float amp, float freq, float offset) :
    amp(amp), freq(freq), offset(offset), time(0.0f){}


bool MockSensor::init(){
    return true;
}

bool MockSensor::update(){
    sample.value++;
    return true;
}

SensorSample MockSensor::get_sample(){
    return this->sample;
}

