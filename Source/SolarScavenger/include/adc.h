#ifndef __ADC_H__
#define __ADC_H__

#include <driver/adc.h>

class AdcReader{
public:
    AdcReader(gpio_num_t _pinAdc);

    void Init();
    uint16_t ReadValue();

private:
    gpio_num_t pinAdc;

};



#endif //__ADC_H__