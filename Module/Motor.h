#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "main.h"
#include "arm_math.h" 
typedef struct   
{

	uint16_t ecd;   
	int16_t  speed_rpm;  
	int16_t  given_current;   
	uint8_t  temperature;   
	int32_t  count; 
	int32_t  all_ecd; 
	int16_t  angle_dif;
//	int32_t real_angle;   
	int16_t  last_ecd;   
	
}motor_measure_t;

#define get_motor_measure(ptr, Data)                                            \
{                                                                               \
    if((ptr)->ecd - (ptr)->last_ecd > 4096) (ptr)->count--;                     \
	else if((ptr)->ecd - (ptr)->last_ecd < -4096 ) (ptr)->count++;              \
    (ptr)->last_ecd      = (ptr)->ecd;                                          \
    (ptr)->ecd           = Data[0] << 8 | Data[1]; \
    (ptr)->speed_rpm     = Data[2] << 8 | Data[3]; \
    (ptr)->given_current = Data[4] << 8 | Data[5]; \
    (ptr)->temperature   = Data[6];                                         	\
    (ptr)->all_ecd       = (ptr)->count*8191 + (ptr)->ecd;                      \
}

typedef struct
{
	int err;
	int p_int,v_int,t_int;						//这里可根据电机数目自行修改，读取三个电机的位置、速度、转矩
	float position,velocity,torque;	//三个电机的位置、速度、转矩解析存储
	float pitch_Target;
	float  all_position;
	int32_t  all_ecd;   //编码器的值(总值)
	int32_t  count;     //编码器计数值
	int16_t last_ecd; 
	int ecd;   //编码器的值(当前值)
  	uint16_t t_mos;
	uint16_t t_rotor;
	//const dam_struct *damiao_get_motor_measure_point;
}damiao_struct;
//达妙电机接受结构体
 #define damiao_get_motor_measure(ptr, rx_data)                                  \
{                                                                              \
    (ptr)->err = (rx_data)[0] >> 4;                                      \
    (ptr)->p_int = ((rx_data)[1] << 8) | (rx_data)[2];         \
    (ptr)->v_int = ((rx_data)[3] << 4) | ((rx_data)[4] >> 4);  \
    (ptr)->t_int = (((rx_data)[4] & 0xF) << 8) | (rx_data)[5]; \
    (ptr)->position = uint_to_float((ptr)->p_int, P_MIN, P_MAX, 16);           \
    (ptr)->velocity = uint_to_float((ptr)->v_int, V_MIN, V_MAX, 12);           \
    (ptr)->torque = uint_to_float((ptr)->t_int, T_MIN, T_MAX, 12);             \
    (ptr)->ecd = ((ptr)->position / PI * 8191)/2;                                  \
    (ptr)->all_ecd = (ptr)->count * 8191 + (ptr)->ecd;                         \
    (ptr)->all_position = (ptr)->count*2*PI + (ptr)->position;                 \
    /* 位置跳变检测 */                                                         \
    if ((ptr)->ecd - (ptr)->last_ecd > 4096)                                   \
    {                                                                          \
        (ptr)->count--;                                                        \
    }                                                                          \
    else if ((ptr)->ecd - (ptr)->last_ecd < -4096)                             \
    {                                                                          \
        (ptr)->count++;                                                        \
    }                                                                          \
    (ptr)->last_ecd = (ptr)->ecd;                                              \
    (ptr)->t_mos = (uint16_t)((rx_data)[6]);                             \
    (ptr)->t_rotor = (uint16_t)((rx_data)[7]);                           \
}

#define Motar_mode 1 //设置模式为何种模式，为0为IMT模式，为1为位置速度模式，为2为速度模式
#define P_MIN -3.1415926	 //位置最小值
#define P_MAX 3.1415926	 //位置最大值
#define V_MIN -30	 //速度最小值
#define V_MAX 30	 //速度最大值
#define KP_MIN 0.0	 //Kp最小值
#define KP_MAX 500.0 //Kp最大值
#define KD_MIN 0.0	 //Kd最小值
#define KD_MAX 5.0	 //Kd最大值
#define T_MIN -10	 //转矩最大值
#define T_MAX 10	 //转矩最小值


#endif

