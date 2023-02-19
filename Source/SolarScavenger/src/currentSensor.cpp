#include "currentSensor.h"

CurrentSensor::CurrentSensor(gpio_num_t SDA, gpio_num_t SCL, uint8_t addr):
    pinSDA(SDA), pinSCL(SCL), i2cAddr(addr)
{
}

void CurrentSensor::Init()
{
}

uint32_t CurrentSensor::readCurrentMa()
{
    return 0;
}