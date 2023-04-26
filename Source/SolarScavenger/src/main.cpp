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
#include "common.h"

#define SIMUL_ON

extern "C" void app_main(void);

// Move to Config.h
#define CALIBRATE_MOTOR 0

static const char* TAG = "Main";

Simul simul;
//Comms comms;
extern Comms gComms;

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
    gComms.Init();
    gComms.activateReception();
    servo.Init();
    Throttle.Init();
    Rudder.Init();
#else
    gComms.Init();
    gComms.activateReception();
    sdCard.Init();
    motor.Init();
    #if CALIBRATE_MOTOR
        vTaskDelay(pdMS_TO_TICKS(2000));
        motor.Init(2000);
        vTaskDelay(pdMS_TO_TICKS(1000));
        motor.setPowerPercentage(0);
    #endif 
    servo.Init();
    LDR.Init();
    Ina.Init();

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
    uint8_t mac_receptor[6]= {MAC_REC};
    uint8_t mac_emisor[6]  = {MAC_EMI};
    
    Init();


#ifdef EMISOR

    printf("Starting as EMISOR\n");

    gComms.addReceiver(mac_receptor);
    commDataTx sendData;

    while(true)
    {
        sendData.rudder = (uint32_t) Rudder.ReadValue()/3.3f;
        sendData.throttle = (uint32_t) Throttle.ReadValue()/3.3f;
        gComms.sendCommData(sendData);
        ESP_LOGE(TAG, "Sending values: %lu, %lu", sendData.rudder, sendData.throttle);
        servo.setPowerPercentage((uint32_t)gRecvCommData.Power/10.0f);
        ESP_LOGE(TAG, "Signal strength: -%d Db", gRecvCommData.SignalDb);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
#else
    printf("Starting as RECEPTOR\n");
    gComms.addReceiver(mac_emisor);


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
        sendData.SignalDb = lastRssiDb;
        gComms.sendCommData(sendData);

        // Check that comms are working, otherwise restart.
        if(!gComms.checkComms())
        {
            sdCard.printf("[ERROR] Restarting ESP32, no messages received\n");
            vTaskDelay(pdMS_TO_TICKS(400));
            esp_restart();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
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
