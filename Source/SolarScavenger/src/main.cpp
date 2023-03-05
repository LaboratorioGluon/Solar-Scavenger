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

#define SIMUL_ON

extern "C" void app_main(void);

// Move to Config.h
#define CALIBRATE_MOTOR 1


static const char* TAG = "Main";

Simul simul;
Comms comms;

#ifdef EMISOR

    AdcReader Throttle(1, ADC_CHANNEL_3);
    AdcReader Rudder(1, ADC_CHANNEL_0);

#else // RECEPTOR
    Mppt mppt;
    RcPwm motor(LEDC_CHANNEL_1, GPIO_NUM_17);
    RcPwm servo(LEDC_CHANNEL_0, GPIO_NUM_16);

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
    //motor.Init();

    // Init ESP-NOW
    // GPIOs
    // Calibrate sensors
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
    
    comms.Init();

    //comms.testGetAddr();

#ifdef EMISOR

    printf("Starting as EMISOR\n");

    Throttle.Init();
    Rudder.Init();

    comms.addReceiver(mac);
    uint8_t data[1] = {0x00};
    commData sendData;
    

    while(true)
    {
        //comms.sendRawData(data, 1);
        sendData.rudder = Rudder.ReadValue();
        sendData.throttle = Throttle.ReadValue();
        comms.sendCommData(sendData);
        ESP_LOGE(TAG, "Sending values: %d, %d", sendData.rudder, sendData.throttle);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
#else
    printf("Starting as RECEPTOR\n");

    comms.activateReception();
    /*#if CALIBRATE_MOTOR
        vTaskDelay(pdMS_TO_TICKS(2000));
        motor.Init(2000);
        vTaskDelay(pdMS_TO_TICKS(1000));
        motor.setPowerPercentage(0);
    #endif */
    motor.Init();
    servo.Init();
    
    while(true)
    {
        servo.setPowerPercentage(gRecvCommData.rudder/33);
        motor.setPowerPercentage(gRecvCommData.throttle/33);
        vTaskDelay(10 / portTICK_PERIOD_MS);
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
