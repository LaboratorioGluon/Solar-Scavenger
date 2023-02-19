#include <stdio.h>
//#include <inttypes.h>
//#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "freertos/queue.h"
//#include "esp_attr.h"
//#include "soc/rtc.h"
//#include "esp_system.h"
//#include "esp_flash.h"
#include "mppt.h"
#include "pwm.h"
#include "simul.h"

#include "motor.h"

#define GPIO_PWM0A_OUT 21   //Set GPIO 21 as PWM0A

extern "C" void app_main(void);

Motor motor(GPIO_NUM_21);

void app_main(void)
{
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

  mcpwm_gpio_initialize(GPIO_PWM0A_OUT);

  uint32_t read_motor_cycle = 80;
  float v = 5, i = 0;

  //Version cleanarda
  /*if( read_motor_cycle < 10 ) stop_motor();
  if( read_motor_cycle < 60 ) set_motor_duty_cycle(read_motor_cycle);
  else set_motor_duty_cycle( mppt_PO(v, i) );*/

  while(true){

      i = simul_I_panel(v);
      read_motor_cycle = mppt_PO(v, i);

      printf("Motor cycle: %d \n", read_motor_cycle);
      printf("   Power: %f \n", v * i);

      set_motor_duty_cycle( read_motor_cycle );
      v = recalculate_V(v, read_motor_cycle);
      printf("   New V: %f New I: %f \n", v, i);

      vTaskDelay(1000 / portTICK_PERIOD_MS);

  }

}
