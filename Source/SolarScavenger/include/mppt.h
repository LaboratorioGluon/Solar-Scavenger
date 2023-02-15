
#include <stdio.h>

#define MAX_DUTY_CYCLE 100
#define MIN_DUTY_CYCLE  20

#define MAX_POWER      100
#define MAX_VOLTAGE   21.6
#define OPT_VOLTAGE   17.8

#define STEP_0           0
#define STEP_1           1
#define STEP_2           2
#define STEP_3           3

#define PERCENT_10_V (MAX_VOLTAGE * 10 ) / 100  // 2,16V
#define PERCENT_20_V (MAX_VOLTAGE * 20 ) / 100  // 4,32V

static float prev_v = 0;
static float prev_i = 0;
static float prev_p = 0;
static bool first_run = true;

static uint32_t duty_cycle;

//TODO -> Ver si se puede samplear cada ciclo o cada mas
//        Lo que hacen en otro es ir sumando el V e I cada X ciclos y luego hacer la media 

//NOTE:
//  Increase voltage = slow speed
//  Decrease voltage = increase speed

uint32_t calculate_step(float power, float v){

    if (power > 95) return STEP_0;

    uint32_t optDiff = abs(v - OPT_VOLTAGE);

    if (optDiff < PERCENT_10_V) return STEP_1;
    if (optDiff < PERCENT_20_V) return STEP_2;
    
    return STEP_3;

}

void limit_cycle(){
    if(duty_cycle > MAX_DUTY_CYCLE) duty_cycle = MAX_DUTY_CYCLE;
    if(duty_cycle < MIN_DUTY_CYCLE) duty_cycle = MIN_DUTY_CYCLE;
}

//Incremental conductance (more computing cost, any benefit?)
//http://ww1.microchip.com/downloads/en/appnotes/00001521a.pdf
uint32_t mppt_IC(float v, float i){
    
    if (first_run){
        prev_v     = v;
        prev_i     = i;
        prev_p     = v * i;
        duty_cycle = 90;
        first_run  = false;
    }

    float p = v * i;
    float Av = v - prev_v;
    uint32_t duty_increment = calculate_step(p, v);

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
    limit_cycle();

    return duty_cycle;
}

//Perturb and observe (more efficient)
//https://github.com/RegataSolar/Solar_MPPT_board/blob/master/MPPT_fw/MPPT_fw.ino
uint32_t mppt_PO(float v, float i){

    if (first_run){
        prev_v     = v;
        prev_i     = i;
        prev_p     = v * i;
        duty_cycle = 90;
        first_run  = false;
    }

    float p = v * i;
    float Ap = p - prev_p;
    float Av = v - prev_v;
    uint32_t duty_increment = calculate_step(p, v);

    if (Ap > 0){

        if(Av < 0) duty_cycle += duty_increment; //Decrease voltage 
        else       duty_cycle -= duty_increment; //Increase voltage

    }else{

        if(Av > 0) duty_cycle += duty_increment; //Decrease voltage
        else       duty_cycle -= duty_increment; //Increase voltage

    }

    prev_v = v;
    prev_p = p;
    limit_cycle();

    return duty_cycle;
}

void reset_mppt(){
    prev_v     = 0;
    prev_i     = 0;
    prev_p     = 0;
    duty_cycle = 0;
    first_run  = true;
}