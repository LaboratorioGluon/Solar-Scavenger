
#if 0
#ifndef __SIMUL_H__
#define __SIMUL_H__

#include <stdint.h>

class Simul{
public:

    Simul();

    float calculateI(float x);
    float recalculateV (float v, uint32_t duty_cycle);

private:

    uint32_t last_cycle;
};

#endif //__SIMUL_H__
#endif