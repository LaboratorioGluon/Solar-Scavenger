#ifndef __SD_H__
#define __SD_H__

#include <driver/sdmmc_host.h>

class SdWritter{
public:
    SdWritter();

    void Init();

    void printf(const char* format, ...);

    void flush();

private:

    FILE *pFile;

    uint8_t isInitialized;
};


#endif //__SD_H__