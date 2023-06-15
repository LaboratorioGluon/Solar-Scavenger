#include "comms.h"

#include "nvs_flash.h"
//#include "esp_random.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_log.h"
//#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "esp_timer.h"

#include <string.h>


#define ESPNOW_CHANNEL 1

static const char* TAG = "Comms";

typedef struct
{
  unsigned frame_ctrl : 16;  // 2 bytes / 16 bit fields
  unsigned duration_id : 16; // 2 bytes / 16 bit fields
  uint8_t addr1[6];          // receiver address
  uint8_t addr2[6];          //sender address
  uint8_t addr3[6];          // filtering address
  unsigned sequence_ctrl : 16; // 2 bytes / 16 bit fields
} wifi_ieee80211_mac_hdr_t;    // 24 bytes

typedef struct
{
  wifi_ieee80211_mac_hdr_t hdr;
  unsigned category_code : 8; // 1 byte / 8 bit fields
  uint8_t oui[3]; // 3 bytes / 24 bit fields
  uint8_t payload[0];
} wifi_ieee80211_packet_t;

struct commDataRx gRecvCommData;

/**
 * Stores the latest received RSSI in DB and unsigned (despite the measuring is negative).
 * Usually range from 0 - 120, being 0 the best signal qualitiy and 120 the worst.
*/
uint8_t lastRssiDb = 0;

Comms gComms;

// Reception Callback
void receptionCallback(const esp_now_recv_info *mac_addr, const uint8_t *data, int data_len)
{
    //ESP_LOGE(TAG, "Nuevos datos recibidos %d > %u", data_len, data[0]);

    memcpy(&gRecvCommData, data, sizeof(struct commDataRx));
    
    gComms.newMessageArrived();
}

void receptionCallbackPromisc(void *buf, wifi_promiscuous_pkt_type_t type)
{
    // Get only management frames
    if (type != WIFI_PKT_MGMT)
        return;
    
    const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
    const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;

    static const uint8_t esp_oui[3] = {0x18, 0xfe, 0x34}; // esp32 oui

    // Filter vendor specific frame with the esp oui.
    if (((ipkt->category_code) == 127) && (memcmp(ipkt->oui, esp_oui,3) == 0))
    {
        lastRssiDb = (uint8_t) abs(ppkt->rx_ctrl.rssi);
        //ESP_LOGE(TAG, "Nuevo mgm frame received -> %d dbm", ppkt->rx_ctrl.rssi);  // For debug pourposes
    }
}

Comms::Comms()
{
    lastMessageMicros=0;
    lastRssiDb = 0;
    memset(&gRecvCommData, 0, sizeof(commDataRx));
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

    error = esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR);
    
    if (error != ESP_OK)
        return error;

    error = esp_wifi_set_promiscuous(true);
    
    if (error != ESP_OK)
        return error;

    error = esp_now_init();

    if (error != ESP_OK)
        return error;

    error = esp_wifi_set_max_tx_power(84);
    return error;
}

void Comms::newMessageArrived()
{
    lastMessageMicros = esp_timer_get_time();
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

void Comms::sendCommData(struct commDataTx data)
{
    sendRawData((uint8_t*)&data, sizeof(struct commDataTx));
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

    #ifndef MANDO
    /**
     * We only want to measure the signal in the receiver side (as tested, signal strength is no symmetrical)
     * as is the more critical side.
     * To measure on both sides, uncomment define. 
     */
    esp_wifi_set_promiscuous_rx_cb(&receptionCallbackPromisc);
    #endif
}

int Comms::checkComms()
{
    if ( (esp_timer_get_time() - lastMessageMicros) > 3000000ULL )
    {
        return 0;
    }
    return 1;
}

void Comms::testGetAddr()
{
    uint8_t mac[6];
    Init();

    //esp_wifi_get_mac(WIFI_IF_STA,mac);
    //esp_read_mac(mac,ESP_MAC_WIFI_STA);
    //ESP_LOGE(TAG, "MAC: %d:%d:%d:%d:%d:%d\n",mac[0], mac[1], mac[2],mac[3],mac[4],mac[5]);

    while(true);
}