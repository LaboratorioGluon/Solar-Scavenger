#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mppt.h"
#include "simul.h"
#include "rcpwm.h"
#include "comms.h"
#include "secret.h"
#include "adc.h"
#include "esp_log.h"
#include "sd.h"
#include "currentSensor.h"

#define SIMUL_ON

extern "C" void app_main(void);

// Move to Config.h
#define CALIBRATE_MOTOR 1


static const char* TAG = "Main";

Simul simul;
Comms comms;

#ifdef EMISOR

    AdcReader Throttle(1, ADC_CHANNEL_7);
    AdcReader Rudder(1, ADC_CHANNEL_6);
    RcPwm servo(LEDC_CHANNEL_0, GPIO_NUM_16, true);

#else // RECEPTOR
    Mppt mppt;
    RcPwm motor(LEDC_CHANNEL_1, GPIO_NUM_21);
    RcPwm servo(LEDC_CHANNEL_0, GPIO_NUM_16, true);

    SdWritter sdCard;

    AdcReader LDR(1, ADC_CHANNEL_3);
    CurrentSensor Ina(GPIO_NUM_22, GPIO_NUM_23, 0x45);

#endif 





void Restart()
{
    esp_restart();
}

void Error()
{
    // Parar motor

    // Parpadear
    while(true){
        //LED rojo ON
        vTaskDelay(pdMS_TO_TICKS(500));
        //LED rojo OFF
        vTaskDelay(pdMS_TO_TICKS(500));
    }

}

void Init()
{
#if EMISOR
    comms.Init();
    comms.activateReception();
    servo.Init();
    Throttle.Init();
    Rudder.Init();
#else
    comms.Init();
    //comms.testGetAddr();
    comms.activateReception();
    sdCard.Init();
    motor.Init();
    servo.Init();
    LDR.Init();
    Ina.Init();

    sdCard.printf("Hola mundo!...\n");
    gpio_config_t leds;
    leds.intr_type = GPIO_INTR_DISABLE;
    leds.mode = GPIO_MODE_OUTPUT;
    leds.pin_bit_mask = (1 << GPIO_NUM_5) | (1 << GPIO_NUM_27);
    leds.pull_down_en = GPIO_PULLDOWN_DISABLE;
    leds.pull_up_en = GPIO_PULLUP_DISABLE;
    
    gpio_config(&leds);
    gpio_set_level(GPIO_NUM_5, 0);
    gpio_set_level(GPIO_NUM_27, 0);
#endif
}

void Wait()
{
    // Busy waiting for the ESP-NOW command to start
}

void Sail()
{


}



void app_main(void)
{
    uint8_t mac[6]= {MAC_DST};
    
    Init();


#ifdef EMISOR

    printf("Starting as EMISOR\n");

    comms.addReceiver(mac);
    uint8_t data[1] = {0x00};
    commDataTx sendData;
    

    while(true)
    {
        sendData.rudder = (uint32_t) Rudder.ReadValue()/3.3f;
        sendData.throttle = (uint32_t) Throttle.ReadValue()/3.3f;
        comms.sendCommData(sendData);
        ESP_LOGE(TAG, "Sending values: %lu, %lu", sendData.rudder, sendData.throttle);
        servo.setPowerPercentage((uint32_t)gRecvCommData.Power/10.0f);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
#else
    printf("Starting as RECEPTOR1\n");
    comms.addReceiver(mac);

    /*#if CALIBRATE_MOTOR
        vTaskDelay(pdMS_TO_TICKS(2000));
        motor.Init(2000);
        vTaskDelay(pdMS_TO_TICKS(1000));
        motor.setPowerPercentage(0);
    #endif */
    struct commDataTx sendData;
    uint32_t power = 0;
    while(true){
        servo.setPowerPercentage(gRecvCommData.throttle/10);
        if (power < 1000){
            power += 50;
        }else{
            power = 0;
        }
        sendData.Power = power;
        comms.sendCommData(sendData);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    uint8_t ledValue = 0;
    uint8_t nLedValue = 1;

    uint32_t startvalue = LDR.ReadValue();
    printf("Start value: %lu \n", startvalue);
    uint32_t lastValue;

    while(true)
    {
        /*servo.setPowerPercentage(gRecvCommData.rudder/33);
        motor.setPowerPercentage(gRecvCommData.throttle/33);
        vTaskDelay(10 / portTICK_PERIOD_MS);*/
        /*gpio_set_level(GPIO_NUM_5,ledValue);
        gpio_set_level(GPIO_NUM_27,nLedValue);
        ledValue = ledValue?0:1;
        nLedValue = nLedValue?0:1;*/

        lastValue = LDR.ReadValue();
        if ( lastValue > startvalue*2 )
        {
            gpio_set_level(GPIO_NUM_5  ,1);
            gpio_set_level(GPIO_NUM_27 ,0);
            servo.setPowerPercentage(0);
            motor.setPowerPercentage(10);
        }
        else
        {
            gpio_set_level(GPIO_NUM_5  ,0);
            gpio_set_level(GPIO_NUM_27 ,1);
            servo.setPowerPercentage(100);
            motor.setPowerPercentage(0);
        }
        Ina.readCurrentMa();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
#endif
#if 0
    Init();
    Wait();
    Sail();

    printf("Start\n");
    motor.Init();
    uint32_t powerLevel = 0;
    int8_t direction = 1; // o -1 para atras
    while (true)
    {
        powerLevel += direction;
        if (powerLevel >= 100)
        {
            direction = -1;
        }
        else if (powerLevel <= 0)
        {
            direction = 1;
        }
        motor.setPowerPercentage(powerLevel);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    return;

    uint32_t read_motor_cycle = 80;
    float v = 5, i = 0;

#ifdef SIMUL_ON

    while (true)
    {
        i = simul.calculateI(v);
        read_motor_cycle = mppt.mpptPO(v, i);

        printf("Motor cycle: %d \n", read_motor_cycle);
        printf("   Power: %f \n", v * i);

        v = simul.recalculateV(v, read_motor_cycle);
        printf("   New V: %f New I: %f \n", v, i);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

#else

    while (true)
    {
        if (read_motor_cycle < 60)
            set_motor_duty_cycle(read_motor_cycle);
        else
            set_motor_duty_cycle(mppt.mpptPO(v, i));
    }

#endif
#endif
}
