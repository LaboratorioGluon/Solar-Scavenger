#include "sd.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "esp_vfs.h"

static const char *TAG = "SD";

#define MOUNT_POINT "/sdcard"
#define LOG_FILENAME "log2.txt"

SdWritter::SdWritter()
{

}

void SdWritter::Init()
{

    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_PROBING;
    host.flags = SDMMC_HOST_FLAG_1BIT;
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 1;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    /*slot_config.gpio_cd = GPIO_NUM_33;*/


    /*gpio_pulldown_dis(GPIO_NUM_2);
    gpio_pullup_en(GPIO_NUM_2);*/

    //esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    pFile = fopen(MOUNT_POINT "/" LOG_FILENAME, "w");
    this->printf("Probando...\n");

}

void SdWritter::printf(const char *format, ...)
{
    va_list args;
    int ret;
    uint8_t failed = 1;
    if (pFile == NULL)
    {
        // ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    va_start(args, format);
    ret = vfprintf(pFile, format, args);

    va_end(args);
    if ( ret > 0){
        ret = fflush(pFile);
        if (ret  == 0 )
        {
            ret = fsync(fileno(pFile));
            if (ret == 0)
            {
                failed = 0;
            }
        }
    }
    

    if (failed)
    {
        fclose(pFile);
        pFile = fopen(MOUNT_POINT "/" LOG_FILENAME, "w");
    }
}
