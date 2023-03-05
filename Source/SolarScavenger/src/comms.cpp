#include "comms.h"

#include "nvs_flash.h"
//#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
//#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"

#include <string.h>


#define ESPNOW_CHANNEL 1

static const char* TAG = "Comms";

struct commData gRecvCommData = {0};

// Reception Callback
void receptionCallback(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    ESP_LOGE(TAG, "Nuevos datos recibidos %d > %d", data_len, data[0]);

    memcpy(&gRecvCommData, data, sizeof(struct commData));
    ESP_LOGE(TAG,"Datos recibidos en commData: rud: %d, thr: %d", gRecvCommData.rudder, gRecvCommData.throttle);


    
}



Comms::Comms()
{

}

esp_err_t Comms::Init()
{

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    
    esp_err_t error = ESP_OK;
    error = esp_netif_init();
    if (error != ESP_OK)
        return error;

    error = esp_event_loop_create_default();
    if (error != ESP_OK)
        return error;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    error = esp_wifi_init(&cfg);
    if (error != ESP_OK)
        return error;

    error = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (error != ESP_OK)
        return error;

    error = esp_wifi_set_mode(WIFI_MODE_STA);
    if (error != ESP_OK)
        return error;

    error = esp_wifi_start();
    if (error != ESP_OK)
        return error;

    error = esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
    if (error != ESP_OK)
        return error;

    error = esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);
    
    if (error != ESP_OK)
        return error;
    error = esp_now_init();
    return error;
}

void Comms::addReceiver(uint8_t receiver_mac[6])
{
    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(esp_now_peer_info_t));
    peer.channel = ESPNOW_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;
    memcpy(peer.peer_addr, receiver_mac, ESP_NOW_ETH_ALEN);
    esp_now_add_peer(&peer);
    memcpy(receiverMac, receiver_mac, ESP_NOW_ETH_ALEN);
}

void Comms::sendCommData(struct commData data)
{
    sendRawData((uint8_t*)&data, sizeof(struct commData));
}

void Comms::sendRawData(uint8_t *data, uint32_t data_len)
{
    if (esp_now_send(receiverMac, data, data_len) != ESP_OK) {
        ESP_LOGE(TAG, "Send error");
    }
}
void Comms::activateReception()
{
    esp_now_register_recv_cb(receptionCallback);
}

void Comms::testGetAddr()
{
    uint8_t mac[6];
    Init();

    //esp_wifi_get_mac(WIFI_IF_STA,mac);
    esp_read_mac(mac,ESP_MAC_WIFI_STA);
    ESP_LOGE(TAG, "MAC: %d:%d:%d:%d:%d:%d\n",mac[0], mac[1], mac[2],mac[3],mac[4],mac[5]);

    while(true);
}