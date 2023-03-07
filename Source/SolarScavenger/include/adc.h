#ifndef __ADC_H__
#define __ADC_H__

//#include <driver/adc.h>
#include <esp_adc/adc_continuous.h>

class AdcReader{
public:
    AdcReader(uint32_t _adcNumber,adc_channel_t _channelAdc);

    void Init();
    uint32_t ReadValue(uint32_t samples = 5);

    static bool calibrateAdc();

private:

    adc_channel_t channelAdc;
    uint32_t adcNumber;

};



#endif //__ADC_H__