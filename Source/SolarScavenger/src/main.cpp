#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mppt.h"
#include "simul.h"
#include "motor.h"

#define SIMUL_ON

extern "C" void app_main(void);

Motor motor(GPIO_NUM_21);
Mppt mppt;
Simul simul;

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
    motor.Init();

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
}
