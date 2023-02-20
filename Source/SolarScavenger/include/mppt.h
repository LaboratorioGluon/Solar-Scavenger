#ifndef __MPPT_H__
#define __MPPT_H__

#include <stdint.h>

class Mppt{
public:

    Mppt();

    uint32_t mpptPO(float v, float i);
    uint32_t mpptIC(float v, float i);

    void resetMppt();

private:
   
    uint32_t calculateStep(float power, float v);
    void limitCycle();

    float prev_v;
    float prev_i;
    float prev_p;
    bool first_run;

    uint32_t duty_cycle;

};

#endif //__MPPT_H__