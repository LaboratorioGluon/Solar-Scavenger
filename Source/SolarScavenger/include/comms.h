#ifndef __COMMS_H__
#define __COMMS_H__

#include <stdint.h>
#include "esp_err.h"

#ifdef EMISOR
struct commDataTx
#else 
struct commDataRx
#endif 
{
    uint32_t rudder; // [0-1000]: 0 - Babor, 500 - Centro,  1000 - Estribor
    uint32_t throttle; // [ 0 - 1000]: 0 - Parado, 1000-  A toa' leche
};

#ifdef EMISOR
struct commDataRx
#else 
struct commDataTx
#endif 
{
    uint32_t Power;     // [0-1000] * 0.1W 
    uint32_t BattLevel; // [3500 - 4900] * 1mV
};

extern struct commDataRx gRecvCommData;

class Comms
{
public:
    Comms();

    esp_err_t Init();
    void addReceiver(uint8_t receiver_mac[6]);

    void sendCommData(struct commDataTx data);
    void sendRawData(uint8_t *data, uint32_t data_len);

    void activateReception();

    // Run to obtain MAC address
    void testGetAddr();

private:
    uint8_t isInitialized;

    uint8_t receiverMac[6];

};

#endif // __COMMS_H__