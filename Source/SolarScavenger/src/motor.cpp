#include "motor.h"

#include "esp_log.h"
static const char* TAG = "Motor";

Motor::Motor(gpio_num_t _pinEsc):isInitialized(false), pinEsc(_pinEsc)
{

}

void Motor::setMicrosecondsUp(uint32_t microseconds)
{
    uint32_t duty = microseconds * ((1<<13) - 1)/20000;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

/* Configure the PWM to 50Hz */
void Motor::Init()
{
    esp_err_t error;
    ledc_timer_config_t ledc_timer;


    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num = LEDC_TIMER_0;
    ledc_timer.duty_resolution = LEDC_TIMER_13_BIT;
    ledc_timer.freq_hz = 50;
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;
    error = ledc_timer_config(&ledc_timer);
    if (error != ESP_OK)
        return;

    ledc_channel_config_t ledc_channel;
    
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc_channel.timer_sel = LEDC_TIMER_0;
    ledc_channel.intr_type = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num = GPIO_NUM_21;
    ledc_channel.duty = 0;
    ledc_channel.hpoint = 0;

    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    setMicrosecondsUp(1000);

    isInitialized = true;
}

void Motor::setPowerPercentage(uint32_t power)
{
    uint32_t microseconds = power*10+1000;
    setMicrosecondsUp(microseconds);
}