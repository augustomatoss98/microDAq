#include <unity.h>
#include <cstdio>
#include "SensorManager/SensorManager.h"
#include "ISensor/ISensor.h"


class MockSensor : public ISensor{
public:
    bool init() override{
        return true;
    }

    bool set_value(uint16_t value){
        this->sample.value = value;
        return true;
    }

    bool update() {
        this->sample.value++;
        return true;
    }

    SensorSample get_sample() override {
        return this->sample;
    }

private:
    SensorSample sample;

};

void setUp(){};

void tearDown(){};

void test_add_sensor(){
    SensorManager sensor_mng;
    MockSensor mock1;
    sensor_mng.add_sensor(1, mock1);

    TEST_ASSERT_EQUAL_UINT8(1, sensor_mng.get_sensor_count());
};

void test_add_sensor_limit(){
    SensorManager sensor_mng;
    MockSensor mock;

    for (uint8_t i = 0; i < 8; i++){
        sensor_mng.add_sensor(i, mock);
    }

    TEST_ASSERT_EQUAL_UINT8(8, sensor_mng.get_sensor_count());

    TEST_ASSERT_FALSE(sensor_mng.add_sensor(9, mock));
    TEST_ASSERT_EQUAL_UINT8(8, sensor_mng.get_sensor_count());

}

void test_process_single_sensor(){
    SensorManager sensor_mng;
    MockSensor mock;
    SensorSample sample;

    sensor_mng.add_sensor(1, mock);
    sensor_mng.process();
    sensor_mng.read(1, sample);

    TEST_ASSERT_EQUAL_UINT8(1, sample.value);

    sensor_mng.process();
    sensor_mng.read(1, sample);

    TEST_ASSERT_EQUAL_UINT8(2, sample.value);

    sensor_mng.process();
    sensor_mng.read(1, sample);

    TEST_ASSERT_EQUAL_UINT8(3, sample.value);
}

void test_process_multiple_sensor(){
    SensorManager sensor_mng;
    MockSensor mock1, mock2, mock3;
    SensorSample sample1, sample2, sample3;

    mock1.set_value(10);
    mock2.set_value(20);
    mock3.set_value(30);

    sensor_mng.add_sensor(1, mock1);
    sensor_mng.add_sensor(2, mock2);
    sensor_mng.add_sensor(3, mock3);

    sensor_mng.process();
    
    sensor_mng.read(1, sample1);
    TEST_ASSERT_EQUAL_UINT8(11, sample1.value);

    sensor_mng.read(2, sample2);
    TEST_ASSERT_EQUAL_UINT8(21, sample2.value);

    sensor_mng.read(3, sample3);
    TEST_ASSERT_EQUAL_UINT8(31, sample3.value);

}

void test_process_failed_read(){
    SensorManager sensor_mng;
    MockSensor mock1, mock2;
    SensorSample sample;

    sensor_mng.add_sensor(1, mock1);
    sensor_mng.add_sensor(2, mock2);

    TEST_ASSERT_FALSE(sensor_mng.read(3, sample));
}

int main(){
    UNITY_BEGIN();

    RUN_TEST(test_add_sensor);
    RUN_TEST(test_add_sensor_limit);
    RUN_TEST(test_process_single_sensor);
    RUN_TEST(test_process_multiple_sensor);
    RUN_TEST(test_process_failed_read);

    return UNITY_END();
};