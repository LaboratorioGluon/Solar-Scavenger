
#include "driver/adc.h"
#include <sys/random.h>
#include <math.h>

#define num_E   2.718
#define V_step  1.0

static uint32_t last_cycle = 90;

//Intriduciendo un voltaje, te da la intensidad de salida del panel
float simul_I_panel(float x){
    float res = (5.8 - 0.000000004 * pow(num_E,x));
    if (res < 0 ) res = 0.1;
    if (res > 6 ) res = 6.0;
    return res;
}

//En funcion del ciclo de trabajo del motor, varia el voltaje
float recalculate_V (float v, uint32_t duty_cycle){

    float result   = v;
    int cycle_diff = duty_cycle - last_cycle;

    if(cycle_diff > 0 ) result -= V_step;
    if(cycle_diff < 0 ) result += V_step;

    //Variaciones aleatorias
    result += (esp_random() % 10) / 5.0;
    result -= (esp_random() % 10) / 5.0;

    if (result < 0 )   result = 0;
    if (result > 21.6) result = 21.6;

    last_cycle = duty_cycle;

    return result;
}