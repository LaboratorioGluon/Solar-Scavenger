#ifndef __ADC_H__
#define __ADC_H__

#include <driver/adc.h>

class AdcReader{
public:
    AdcReader();

    void Init();
    uint16_t ReadValue();

private:


};



#endif //__ADC_H__