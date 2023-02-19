#ifndef __CURRENT_SENSOR_H__
#define __CURRENT_SENSOR_H__

#include <driver/i2c.h>

class CurrentSensor{
public:
    CurrentSensor(gpio_num_t SDA, gpio_num_t SCL, uint32_t addr);

    void Init();

    uint32_t readCurrentMa();
};


#endif //__CURRENT_SENSOR_H__