#pragma once

#include "SensorSample/SensorSample.h"

class ISensor{
public:
    virtual ~ISensor() = default;

    virtual bool init() = 0;
    virtual bool update() = 0;
    virtual SensorSample get_sample() = 0;
};