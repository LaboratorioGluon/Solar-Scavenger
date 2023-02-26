#ifndef __COMMS_H__
#define __COMMS_H__

#include <stdint.h>
#include "esp_err.h"

class Comms
{
public:
    Comms();

    esp_err_t Init();
    void addReceiver(uint8_t receiver_mac[6]);

    void sendData(uint8_t *data, uint32_t data_len);

    void activateReception();

    // Run to obtain MAC address
    void testGetAddr();

private:
    uint8_t isInitialized;

    uint8_t receiverMac[6];

};

#endif // __COMMS_H__