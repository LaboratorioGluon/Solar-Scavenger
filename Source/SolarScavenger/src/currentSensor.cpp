#include "currentSensor.h"
#include "esp_log.h"

static const char *TAG = "CURRENT_SENSOR";

CurrentSensor::CurrentSensor(gpio_num_t SDA, gpio_num_t SCL, uint8_t addr):
    pinSDA(SDA), pinSCL(SCL), i2cAddr(addr)
{
    
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = pinSDA;         // select SDA GPIO specific to your project
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = pinSCL;         // select SCL GPIO specific to your project
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;  // select frequency specific to your project
    conf.clk_flags = 0;                          
    
}

esp_err_t CurrentSensor::Init()
{

    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);

    uint8_t reg = 0;
    uint8_t read[2]={0};
    esp_err_t ret;
    ret = i2c_master_write_read_device( i2c_master_port, 0x45, &reg, 1, read, 2, pdMS_TO_TICKS(1000));
    ESP_LOGE(TAG, "Read (%d) : 0x%x 0x%x",ret, read[0], read[1]);

    // @TODO: Hacer un check de que hemos leido bien
    return ret;
}

uint32_t CurrentSensor::readCurrentMa()
{
    uint8_t reg = 1;
    uint8_t read[2]={0};
    i2c_master_write_read_device( i2c_master_port, 0x45, &reg, 1, read, 2, pdMS_TO_TICKS(1000));
    uint32_t valueMa = (read[0]<<8 | read[1])/5;
    ESP_LOGE(TAG, "Read : 0x%x 0x%x -> %lu", read[0], read[1], valueMa);
    return valueMa;
}