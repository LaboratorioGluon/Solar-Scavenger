#if 0
#include "simul.h"
#include <math.h>

#define num_E   2.718f
#define V_step  1.0f

Simul::Simul():last_cycle(90){
}

//Introduciendo un voltaje, te da la intensidad de salida del panel
float Simul::calculateI(float x){
    float res = (5.8f - 0.000000004f * pow(num_E,x));
    if (res < 0 ) res = 0.1f;
    if (res > 6 ) res = 6.0f;
    return res;
}

//En funcion del ciclo de trabajo del motor, varia el voltaje
float Simul::recalculateV (float v, uint32_t duty_cycle){

    float result   = v;
    int cycle_diff = duty_cycle - last_cycle;

    if(cycle_diff > 0 ) result -= V_step;
    if(cycle_diff < 0 ) result += V_step;

    //Variaciones aleatorias
    result += (random() % 10) / 5.0f;
    result -= (random() % 10) / 5.0f;

    if (result < 0 )    result =  0.0f;
    if (result > 21.6f) result = 21.6f;

    last_cycle = duty_cycle;

    return result;
}

#endif