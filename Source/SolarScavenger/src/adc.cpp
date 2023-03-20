#include "adc.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

static const char *TAG = "ADC";

// TODO: Implementar calibracion
static bool calibrateAdc()
{
    return false;
}

AdcReader::AdcReader(uint32_t _adcNumber,adc_channel_t _channelAdc)
    :channelAdc(_channelAdc), adcNumber(_adcNumber)
{

}

void AdcReader::Init()
{
    if( adcNumber == 1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten((adc1_channel_t)channelAdc, ADC_ATTEN_DB_11);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)channelAdc, ADC_ATTEN_DB_11);
    }
    
}

uint32_t AdcReader::ReadValue(uint32_t samples)
{
    uint32_t rawValue = 0;
    uint32_t sumValue = 0;
    
    for( int i = 0; i < samples; i++)
    {
        if(adcNumber == 1){
            rawValue = adc1_get_raw((adc1_channel_t)channelAdc);
        }else{
            adc2_get_raw((adc2_channel_t)channelAdc, ADC_WIDTH_BIT_12, (int*)&rawValue);
        }
        sumValue += rawValue;
    }
    rawValue = sumValue / samples;
    
    uint32_t valueMv = (3300.0f/(1.0f*(1<<12))) * rawValue;
    //ESP_LOGE(TAG, "ADC raw: %lu -> %lu mV.", rawValue, valueMv);

    return valueMv;
}