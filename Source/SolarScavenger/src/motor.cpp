#include "motor.h"


Motor::Motor(gpio_num_t _pinEsc):isInitialized(false), pinEsc(_pinEsc)
{

}

void Motor::Init()
{
    isInitialized = true;
}

void Motor::setPowerPercentage(uint32_t power)
{

}