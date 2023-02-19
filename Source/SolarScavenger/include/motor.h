#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <driver/gpio.h>
#include <driver/ledc.h>

/**
 * @brief This class controls the ESC which controls the Brushless motor.
 * 
 */
class Motor{
public:
    Motor(gpio_num_t pinEsc);

    void Init();

    void setPowerPercentage(uint32_t power);
private:

    uint8_t isInitialized;

};


#endif //__MOTOR_H__