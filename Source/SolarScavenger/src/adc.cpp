#include "adc.h"

AdcReader::AdcReader(gpio_num_t _pinAdc):pinAdc(_pinAdc)
{
}

void AdcReader::Init()
{
}

uint16_t AdcReader::ReadValue()
{
    return 0;
}