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
#define CALIBRATE_MOTOR 1
#define LED_RED_PIN    GPIO_NUM_27 
#define LED_GREEN_PIN  GPIO_NUM_5
#define CMD_MPPT_LIMIT 850

uint8_t isFreshStart;

static const char* TAG = "Main";

//Simul simul;
extern Comms gComms;

#ifdef MANDO

    AdcReader Throttle(1, ADC_CHANNEL_7);
    AdcReader Rudder(1, ADC_CHANNEL_6);
    RcPwm servoPower(LEDC_CHANNEL_0, GPIO_NUM_18, true);
    RcPwm servoSignal(LEDC_CHANNEL_1, GPIO_NUM_17, true);
    RcPwm servoBattery(LEDC_CHANNEL_2, GPIO_NUM_16, true);

#else // BARCO
    Mppt mppt;
    RcPwm motor(LEDC_CHANNEL_1, GPIO_NUM_21);
    RcPwm servo(LEDC_CHANNEL_0, GPIO_NUM_16, true);

    SdWritter sdCard;

    AdcReader LDR(1, ADC_CHANNEL_3);
    AdcReader BatteryLevel(1, ADC_CHANNEL_4);
    AdcReader motorVoltage(1, ADC_CHANNEL_0);
    CurrentSensor Ina(GPIO_NUM_22, GPIO_NUM_23, 0x45);

#endif 


void waitForChar(){
    char b;
    do{
        b = getchar();
        vTaskDelay(pdMS_TO_TICKS(100));
    }while(b == 0xFF);
}



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
    gpio_config_t leds;
    leds.intr_type = GPIO_INTR_DISABLE;
    leds.mode = GPIO_MODE_OUTPUT;
    leds.pin_bit_mask = (1 << LED_GREEN_PIN) | (1 << LED_RED_PIN);
    leds.pull_down_en = GPIO_PULLDOWN_DISABLE;
    leds.pull_up_en = GPIO_PULLUP_DISABLE;
    
    gpio_config(&leds);
    gpio_set_level(LED_GREEN_PIN, 0);
    gpio_set_level(LED_RED_PIN, 0);

#if MANDO
    gComms.Init();
    gComms.activateReception();
    servoPower.Init();
    servoSignal.Init();
    servoPower.Init();
    Throttle.Init();
    servoBattery.Init();
    servoPower.setPowerPercentage(0);
    servoSignal.setPowerPercentage(0);
    servoBattery.setPowerPercentage(0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    servoPower.setPowerPercentage(100);
    servoSignal.setPowerPercentage(100);
    servoBattery.setPowerPercentage(100);
    vTaskDelay(pdMS_TO_TICKS(0));
    servoPower.setPowerPercentage(100);
    servoSignal.setPowerPercentage(100);
    servoBattery.setPowerPercentage(100);
    

#else
    gComms.Init();
    gComms.activateReception();
    sdCard.Init();
    #if CALIBRATE_MOTOR
        vTaskDelay(pdMS_TO_TICKS(2000));
        motor.Init(2000);
        printf("Connect the ESC now!\n");
        printf("Wait for 2 beeps from the ESC and press a key\n");
        waitForChar();
        motor.setPowerPercentage(0);
        printf("Wait for 1,2,3 beeps \n");
        waitForChar();
    #else
        motor.Init();
    #endif 
    servo.Init();
    LDR.Init();
    BatteryLevel.Init();
    motorVoltage.Init();
    Ina.Init();
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

    
    //isFreshStart = (esp_reset_reason() == ESP_RST_POWERON);
    esp_reset_reason_t resetReason;
    resetReason = esp_reset_reason();
    if(resetReason == ESP_RST_POWERON)
    {
        gpio_set_level(LED_GREEN_PIN, 1);
    }
    else if(resetReason == ESP_RST_BROWNOUT)
    {
        gpio_set_level(LED_GREEN_PIN, 1);
        gpio_set_level(LED_RED_PIN, 1);
    }

#ifdef MANDO

    printf("Starting as MANDO\n");

    gComms.addReceiver(mac);
    commDataTx sendData;

    while(true)
    {
        sendData.rudder = (uint32_t) Rudder.ReadValue()/3.3f;
        sendData.throttle = (uint32_t) Throttle.ReadValue()/3.3f;
        gComms.sendCommData(sendData);
        ESP_LOGE(TAG, "Sending values: %lu, %lu", sendData.rudder, sendData.throttle);
        ESP_LOGE(TAG, "Receiving values: Power: %lu, Batt: %lu, Signal: -%d Db", gRecvCommData.Power, gRecvCommData.BattLevel, gRecvCommData.SignalDb );
        servoPower.setPowerPercentage((uint32_t)gRecvCommData.Power/10.0f);
        servoBattery.setPowerPercentage(((uint32_t)gRecvCommData.BattLevel-3500.0f)/14.0f);
        /*if(gComms.checkComms())
        {
            gpio_set_level(LED_GREEN_PIN, 0);
            gpio_set_level(LED_RED_PIN, 1);
        }
        else 
        {
            gpio_set_level(LED_RED_PIN, 0);
            gpio_set_level(LED_GREEN_PIN, 1);
        }*/

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
#else
    printf("Starting as BARCO\n");
    gComms.addReceiver(mac);

    struct   commDataTx sendData;
    uint32_t power = 0;
    int32_t  delta = 2;
    float    current = 0;
    float    emaCurrent = 0;
    float    meanCurrent = 0;
    float    voltage = 0;
    float    emaVoltage = 0;
    float    meanVoltage = 0;
    uint32_t nMeasures = 0;
    uint32_t mpptOuput = 0;

    float alpha = 0.25;

    uint32_t cycle = 0;


    while(true)
    {

        meanCurrent += Ina.readCurrentMa()/1000.0f;
        meanVoltage += (motorVoltage.ReadValue()/0.12f)/1000.0f;
        nMeasures++;

        if(cycle % 20 == 0)
        {
            current = meanCurrent/nMeasures;
            voltage = meanVoltage/nMeasures;

            // EMA Filter
            emaCurrent = current*alpha + emaCurrent*(1-alpha);
            emaVoltage = voltage*alpha + emaVoltage*(1-alpha);

            if (gRecvCommData.throttle > CMD_MPPT_LIMIT)
            {
                mpptOuput = mppt.mpptIC(emaVoltage, emaCurrent);

                ESP_LOGE(TAG, "[MPTT]%.2f;%.2f;%.2f;%.2f;%lu", current,voltage,emaCurrent,emaVoltage,mpptOuput);
                sdCard.printf("[MPTT]%.2f;%.2f;%.2f;%.2f;%lu", current,voltage,emaCurrent,emaVoltage,mpptOuput);

                motor.setPowerPercentage(mpptOuput);
            }
            else
            {
                ESP_LOGE(TAG, "[MANUAL]%lu", gRecvCommData.throttle);
                motor.setPowerPercentage(gRecvCommData.throttle/10);
            }
            

            nMeasures   = 0;
            meanCurrent = 0;
            meanVoltage = 0;
        }

        if(cycle % 20 == 0){
            
            //ESP_LOGE(TAG, "Receiving values: Rudder: %lu, Throt: %lu", gRecvCommData.rudder, gRecvCommData.throttle);
            servo.setPowerPercentage(gRecvCommData.rudder/10.0f);
            motor.setPowerPercentage(gRecvCommData.throttle/10.0f);

            sendData.Power     = (uint32_t)(emaVoltage * emaCurrent) *100.0f; //TODO - quitar el x10, puesto para pruebas
            sendData.BattLevel = (uint32_t) BatteryLevel.ReadValue() * 2.0f;
            sendData.SignalDb  = lastRssiDb;
            gComms.sendCommData(sendData);
        }


        // Check that comms are working, otherwise restart.
        /*if(gComms.checkComms())
        {
            ESP_LOGE(TAG, "Restarting due to missing communications.");
            sdCard.printf("[ERROR] Restarting ESP32, no messages received\n");
            vTaskDelay(pdMS_TO_TICKS(400));
            esp_restart();
        }*/

        vTaskDelay(pdMS_TO_TICKS(10));
        cycle++;
    }

#endif

}
