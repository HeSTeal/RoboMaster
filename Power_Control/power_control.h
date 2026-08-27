#ifndef __POWER_CONTROL_H
#define __POWER_CONTROL_H

#include "main.h"
void Power_Limit_Init(void);
void get_chassis_power_and_buffer(float *buffer,float *powmax);
void chassis_power_control(void);
void Super_power_ctrl(uint8_t Shift_Flag);


#endif
