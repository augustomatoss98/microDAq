#pragma once

#include <math.h>

#include "ISensor/ISensor.h"
#include "SystemTime/SystemTime.h"

class MockSensor : public ISensor {
public:
    MockSensor(float amp=1.0f, float freq=1.0f, float offset=0.0f);

    bool init() override;
    bool update() override;
    SensorSample get_sample() override;;

private:
    float amp;
    float freq;
    float offset;

    float time;

    SensorSample sample;
};