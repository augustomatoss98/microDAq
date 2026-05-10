#include "MockSensor.h"

MockSensor::MockSensor(float amp, float freq, float offset) :
    amp(amp), freq(freq), offset(offset), time(0.0f){}


bool MockSensor::init(){
    return true;
}

bool MockSensor::update(){
    constexpr float PI = 3.14159265f;
    this->time = SystemTime::millis();
    
    this->sample.timestamp = time; 

    this->sample.value = this->offset + this->amp*sinf(2.0*PI*freq*time);

    return true;
}

SensorSample MockSensor::get_sample(){
    return this->sample;
}

