#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

//https://github.com/espressif/esp-idf/blob/v4.3/examples/peripherals/mcpwm/mcpwm_brushed_dc_control/main/mcpwm_brushed_dc_control_example.c

static void mcpwm_gpio_initialize(int GPIO_PWM0A_OUT)
{
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 1000;    //frequency = 1000Hz
  pwm_config.cmpr_a = 0;       //duty cycle of PWMxA = 0%
  pwm_config.cmpr_b = 0;       //duty cycle of PWMxb = 0%
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);   //Configure PWM0A & PWM0B with above settings

}
 
static void set_motor_duty_cycle(float duty_cycle){
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);
}

static void stop_motor(void){
  mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0);
}