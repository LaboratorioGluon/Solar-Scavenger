
#include "mppt.h"
#include <stdlib.h>

#define MAX_DUTY_CYCLE 90
#define MIN_DUTY_CYCLE  10

#define MAX_VOLTAGE   21.6
#define OPT_VOLTAGE   17.8

#define STEP_0           0
#define STEP_1           2
#define STEP_2           2
#define STEP_3           2

#define PERCENT_10_V (MAX_VOLTAGE * 10 ) / 100  // 2,16V
#define PERCENT_20_V (MAX_VOLTAGE * 20 ) / 100  // 4,32V

//TODO -> Ver si se puede samplear cada ciclo o cada mas
//        Lo que hacen en otro es ir sumando el V e I cada X ciclos y luego hacer la media 

//NOTE:
//  Increase voltage = slow speed
//  Decrease voltage = increase speed

Mppt::Mppt():prev_v(0.0f), prev_i(0.0f), prev_p(0.0f), first_run(true), duty_cycle(0)
{
}

uint32_t Mppt::calculateStep(float power, float v){

    if (power > 95.0f) return STEP_0;

    uint32_t optDiff = abs(v - OPT_VOLTAGE);

    if (optDiff < PERCENT_10_V) return STEP_1;
    if (optDiff < PERCENT_20_V) return STEP_2;
    
    return STEP_3;

}

void Mppt::limitCycle(){
    if(duty_cycle > MAX_DUTY_CYCLE) duty_cycle = MAX_DUTY_CYCLE;
    if(duty_cycle < MIN_DUTY_CYCLE) duty_cycle = MIN_DUTY_CYCLE;
}

//Incremental conductance (more computing cost, any benefit?)
//http://ww1.microchip.com/downloads/en/appnotes/00001521a.pdf
uint32_t Mppt::mpptIC(float v, float i){
    
    if (first_run){
        prev_v     = v;
        prev_i     = i;
        prev_p     = v * i;
        duty_cycle = first_duty_cycle;
        first_run  = false;
    }

    float p = v * i;
    float Av = v - prev_v;
    uint32_t duty_increment = calculateStep(p, v);

    if(Av == 0){

        float Ai = i - prev_i;

        if (Ai > 0) duty_cycle -= duty_increment; //Increase voltage   
        if (Ai < 0) duty_cycle += duty_increment; //Decrease voltage

    }else{
        
        float Ap = p - prev_p;
        float Ap_over_Av = Ap / Av;

        if (Ap_over_Av > 0) duty_cycle -= duty_increment; //Increase voltage
        if (Ap_over_Av < 0) duty_cycle += duty_increment; //Decrease voltage
    }

    prev_v = v;
    prev_i = i;
    prev_p = p;
    limitCycle();

    return duty_cycle;
}

//Perturb and observe (more efficient)
//https://github.com/RegataSolar/Solar_MPPT_board/blob/master/MPPT_fw/MPPT_fw.ino
uint32_t Mppt::mpptPO(float v, float i){

    if (first_run){
        prev_v     = v;
        prev_i     = i;
        prev_p     = v * i;
        duty_cycle = first_duty_cycle;
        first_run  = false;
    }

    float p = v * i;
    float Ap = p - prev_p;
    float Av = v - prev_v;
    uint32_t duty_increment = calculateStep(p, v);

    if (Ap > 0){

        if(Av < 0) duty_cycle += duty_increment; //Decrease voltage 
        else       duty_cycle -= duty_increment; //Increase voltage

    }else{

        if(Av > 0) duty_cycle += duty_increment; //Decrease voltage
        else       duty_cycle -= duty_increment; //Increase voltage

    }

    prev_v = v;
    prev_p = p;
    limitCycle();

    return duty_cycle;
}

void Mppt::resetMppt(uint32_t first_duty){
    prev_v     = 0.0f;
    prev_i     = 0.0f;
    prev_p     = 0.0f;
    duty_cycle = 0.0f;
    first_duty_cycle = first_duty<20?20:first_duty;
    first_run  = true;
}