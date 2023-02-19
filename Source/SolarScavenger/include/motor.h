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
    Motor(gpio_num_t _pinEsc);

    void Init();

    void setPowerPercentage(uint32_t power);
private:

    void setMicrosecondsUp(uint32_t miliseconds);

    uint8_t isInitialized;
    gpio_num_t pinEsc;

};


#endif //__MOTOR_H__