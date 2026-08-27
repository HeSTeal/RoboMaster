//#include "power_control.h"
//#include "controller.h"
//#include "judgement_info.h"
//#include "Chassis_task.h"
//#include "Motor.h"
//#include "can.h"

//PID_t buffer_PID;

//float chassis_power_buffer = 0.0f;
//float chassis_max_power = 0;
//float input_power = 0;		 // input power from battery (referee system)
//float toque_coefficient = 1.99688994e-6f; // (20/16384)*(0.3)*(187/3591)/9.55
//float k1 = 1.23e-07f;  					   	// 转速平方项系数(k1)  1.6e-07; 1.33e-07;	1.23e-07;
//float k2 = 1.453e-07f; 						// 电流平方项系数(k2)  1.653e-07; 1.553e-07; 1.453e-07;		
//float constant = 0;
//float scaled_give_power[4];
//float aa=0;
//float nb=50;
//extern ext_power_heat_data_t PowerHeatData;	
//extern ext_robot_status_t RobotStatust;
//extern PID_t M3508_speed_PID[4];
//extern motor_measure_t M3508[4];
//extern uint8_t zimiao_mode;
//extern ext_sentry_info_t SentryInfo;//0x020D
//extern Super_power_t Super_power;//超电
//extern uint8_t flag_tt;
//void Power_Limit_Init(void)
//{
//    //           PID结构体   PID最大输出 积分限幅  误差死区   P   I   D  微分先行系数A  微分先行系数B  输出滤波  微分滤波  最小二乘法样本数（微分）
//    PID_Init_t(&buffer_PID  ,    100    ,  1000  ,     0    , 5 , 0 , 0 ,      0       ,     0       ,     0    ,    0    ,          0         ,0xff);
//}

//void get_chassis_power_and_buffer(float *buffer,float *powmax)
//{
//    *buffer = PowerHeatData.buffer_energy;
//    
//	 if(SentryInfo.sentry_posture==3)
//        *powmax=100;
//     else if(Super_power.volt>13500&&(SentryInfo.sentry_posture==1||SentryInfo.sentry_posture==2)&&flag_tt==1)
//         *powmax=RobotStatust.chassis_power_limit+30;//防御进攻开超电==1
//     else
//         *powmax=RobotStatust.chassis_power_limit;
//     
////第二场
////     if(Super_power.volt>13500&&SentryInfo.sentry_posture!=3)
////         *powmax=RobotStatust.chassis_power_limit+30;//防御进攻开超电
////     else if(SentryInfo.sentry_posture==2)
////         *powmax=RobotStatust.chassis_power_limit;
////     else
////         *powmax=100;
//}

////void chassis_power_control(void)
////{
////	float initial_total_power = 0;
////	float initial_give_power[4]={0,0,0,0}; 

////	get_chassis_power_and_buffer(&chassis_power_buffer, &chassis_max_power);
////	buffer_PID.MaxOut=chassis_max_power;

////	PID_Calculate(&buffer_PID,chassis_power_buffer,60.0f);
////    
////	if(buffer_PID.Output>chassis_power_buffer)
////	{
////		buffer_PID.Output=chassis_power_buffer;
////	}
////    
////	input_power = chassis_max_power - buffer_PID.Output; // Input power floating at maximum power
////	
////	chassis_max_power = input_power;
////	for (uint8_t i = 0; i < 4; i++) // first get all the initial motor power and total motor power
////	{
////		initial_give_power[i] = M3508_speed_PID[i].Output * toque_coefficient * M3508[i].speed_rpm +
////								k2 * M3508[i].speed_rpm * M3508[i].speed_rpm +
////								k1 * M3508_speed_PID[i].Output * M3508_speed_PID[i].Output +
////								constant;
////								
////		if (initial_give_power[i] < 0) // negative power not included (transitory)
////			continue;
////		initial_total_power += initial_give_power[i];
////	}
////	if (initial_total_power > chassis_max_power) // determine if larger than max power
////	{
////		float power_scale = chassis_max_power / initial_total_power;
////		for (uint8_t i = 0; i < 4; i++)
////		{
////			scaled_give_power[i] = initial_give_power[i] * power_scale*0.81f; // get scaled power
////			if (scaled_give_power[i] < 0)
////			{
////				continue;
////			}

////			float b = toque_coefficient * M3508[i].speed_rpm;
////			float c = k2 * M3508[i].speed_rpm * M3508[i].speed_rpm - scaled_give_power[i] + constant;

////			if (M3508_speed_PID[i].Output> 0) // Selection of the calculation formula according to the direction of the original moment
////			{
////				float temp = (-b + sqrt(b * b - 4 * k1 * c)) / (2 * k1);
////				if (temp > 16000)
////				{
////					M3508_speed_PID[i].Output= 16000;
////				}
////				else
////					M3508_speed_PID[i].Output = temp;
////			}
////			else
////			{
////				float temp = (-b - sqrt(b * b - 4 * k1 * c)) / (2 * k1);
////				if (temp < -16000)
////				{
////					M3508_speed_PID[i].Output = -16000;
////				}
////				else
////					M3508_speed_PID[i].Output = temp;
////			}
////		}
////	}
////}
//void chassis_power_control(void)
//{
//	float initial_total_power = 0;
//	float initial_give_power[4]={0,0,0,0}; 

//	get_chassis_power_and_buffer(&chassis_power_buffer, &chassis_max_power);
//	buffer_PID.MaxOut=chassis_max_power;

//	PID_Calculate(&buffer_PID, chassis_power_buffer,60.0f);
//	if(buffer_PID.Output>chassis_power_buffer)
//	{
//		buffer_PID.Output=chassis_power_buffer;
//	}

//	input_power = chassis_max_power - buffer_PID.Output; // Input power floating at maximum power
//	
//	chassis_max_power = input_power;
//	for (uint8_t i = 0; i < 4; i++) // first get all the initial motor power and total motor power
//	{
//		initial_give_power[i] = M3508_speed_PID[i].Output * toque_coefficient * M3508[i].speed_rpm +
//								k1 * M3508[i].speed_rpm * M3508[i].speed_rpm +
//								k2 * M3508_speed_PID[i].Output* M3508_speed_PID[i].Output +
//								constant;
//								
//		if (initial_give_power[i] < 0) // negative power not included (transitory)
//			continue;
//		initial_total_power += initial_give_power[i];
//	}
//	if (initial_total_power > chassis_max_power) // determine if larger than max power
//	{
//		float power_scale = chassis_max_power / initial_total_power;
//		for (uint8_t i = 0; i < 4; i++)
//		{
//			scaled_give_power[i] = initial_give_power[i] * power_scale*0.81f; // get scaled power
//			if (scaled_give_power[i] < 0)
//			{
//				continue;
//			}

//			float b = toque_coefficient * M3508[i].speed_rpm;
//			float c = k2 * M3508[i].speed_rpm * M3508[i].speed_rpm - scaled_give_power[i] + constant;

//			if (M3508_speed_PID[i].Output > 0) // Selection of the calculation formula according to the direction of the original moment
//			{
//				float temp = (-b + sqrt(b * b - 4 * k1 * c)) / (2 * k1);
//				if (temp > 16000)
//				{
//					M3508_speed_PID[i].Output= 16000;
//				}
//				else
//					M3508_speed_PID[i].Output = temp;
//			}
//			else
//			{
//				float temp = (-b - sqrt(b * b - 4 * k1 * c)) / (2 * k1);
//				if (temp < -16000)
//				{
//					M3508_speed_PID[i].Output = -16000;
//				}
//				else
//					M3508_speed_PID[i].Output = temp;
//			}
//		}
//	}
//}

////超电发送
//void Super_power_ctrl(uint8_t Shift_Flag)//0 2充电1放电
//{
//	Can_Send_TO_Superpower(&hcan2,RobotStatust.chassis_power_limit,Shift_Flag,PowerHeatData.buffer_energy);
//}

#include "power_control.h"
#include "controller.h"
#include "judgement_info.h"
#include "Chassis_task.h"
#include "Motor.h"
#include "can.h"
#include "NAVI_Task.h"

PID_t buffer_PID;

float chassis_power_buffer = 0.0f;
float chassis_max_power = 0;
float input_power = 0;		 // input power from battery (referee system)
float toque_coefficient = 1.99688994e-6f; // (20/16384)*(0.3)*(187/3591)/9.55
float k1 = 1.23e-07f;  					   	// 转速平方项系数(k1)  1.6e-07; 1.33e-07;	1.23e-07;
float k2 = 1.453e-07f; 						// 电流平方项系数(k2)  1.653e-07; 1.553e-07; 1.453e-07;		
float constant = 0.0f;
float scaled_give_power[4];
extern ext_power_heat_data_t PowerHeatData;	
extern ext_robot_status_t RobotStatust;
extern PID_t M3508_speed_PID[4];
extern motor_measure_t M3508[4];
extern uint8_t zimiao_mode;
extern ext_sentry_info_t SentryInfo;//0x020D
extern Super_power_t Super_power;//超电
extern uint8_t flag_tt;
extern NAVI_RX_union NAVI_Data;
extern ext_game_status_t GameState;
void Power_Limit_Init(void)
{
    //           PID结构体   PID最大输出 积分限幅  误差死区   P   I   D  微分先行系数A  微分先行系数B  输出滤波  微分滤波  最小二乘法样本数（微分）
    PID_Init_t(&buffer_PID  ,    100    ,  1000  ,     0    , 5 , 0 , 0 ,      0       ,     0       ,     0    ,    0    ,          0         ,0xff);
}

void get_chassis_power_and_buffer(float *buffer,float *powmax)
{
    *buffer = PowerHeatData.buffer_energy;
     
    if(GameState.game_progress!=0)
    {
        if(SentryInfo.sentry_posture==3)
            *powmax=80;
        else if(Super_power.volt>13500&&(SentryInfo.sentry_posture==1||SentryInfo.sentry_posture==2)&&flag_tt==1)
            *powmax=RobotStatust.chassis_power_limit+30;//防御进攻开超电==1
        else if(NAVI_Data.NAVI_RX_Data.chassis_type==1&&RobotStatust.current_HP>380)
            *powmax=100;
        else
            *powmax=RobotStatust.chassis_power_limit;
    }
	else
        *powmax=RobotStatust.chassis_power_limit;
}

//void chassis_power_control(void)
//{
//	float initial_total_power = 0;
//	float initial_give_power[4]={0,0,0,0}; 

//	get_chassis_power_and_buffer(&chassis_power_buffer, &chassis_max_power);
//	buffer_PID.MaxOut=chassis_max_power;

//	PID_Calculate(&buffer_PID,chassis_power_buffer,60.0f);
//    
//	if(buffer_PID.Output>chassis_power_buffer)
//	{
//		buffer_PID.Output=chassis_power_buffer;
//	}
//    
//	input_power = chassis_max_power - buffer_PID.Output; // Input power floating at maximum power
//	
//	chassis_max_power = input_power;
//	for (uint8_t i = 0; i < 4; i++) // first get all the initial motor power and total motor power
//	{
//		initial_give_power[i] = M3508_speed_PID[i].Output * toque_coefficient * M3508[i].speed_rpm +
//								k2 * M3508[i].speed_rpm * M3508[i].speed_rpm +
//								k1 * M3508_speed_PID[i].Output * M3508_speed_PID[i].Output +
//								constant;
//								
//		if (initial_give_power[i] < 0) // negative power not included (transitory)
//			continue;
//		initial_total_power += initial_give_power[i];
//	}
//	if (initial_total_power > chassis_max_power) // determine if larger than max power
//	{
//		float power_scale = chassis_max_power / initial_total_power;
//		for (uint8_t i = 0; i < 4; i++)
//		{
//			scaled_give_power[i] = initial_give_power[i] * power_scale*0.81f; // get scaled power
//			if (scaled_give_power[i] < 0)
//			{
//				continue;
//			}

//			float b = toque_coefficient * M3508[i].speed_rpm;
//			float c = k2 * M3508[i].speed_rpm * M3508[i].speed_rpm - scaled_give_power[i] + constant;

//			if (M3508_speed_PID[i].Output> 0) // Selection of the calculation formula according to the direction of the original moment
//			{
//				float temp = (-b + sqrt(b * b - 4 * k1 * c)) / (2 * k1);
//				if (temp > 16000)
//				{
//					M3508_speed_PID[i].Output= 16000;
//				}
//				else
//					M3508_speed_PID[i].Output = temp;
//			}
//			else
//			{
//				float temp = (-b - sqrt(b * b - 4 * k1 * c)) / (2 * k1);
//				if (temp < -16000)
//				{
//					M3508_speed_PID[i].Output = -16000;
//				}
//				else
//					M3508_speed_PID[i].Output = temp;
//			}
//		}
//	}
//}
void chassis_power_control(void)
{
	float initial_total_power = 0;
	float initial_give_power[4]={0,0,0,0}; 

	get_chassis_power_and_buffer(&chassis_power_buffer, &chassis_max_power);
	buffer_PID.MaxOut=chassis_max_power;

	PID_Calculate(&buffer_PID, chassis_power_buffer,60.0f);
	if(buffer_PID.Output>chassis_max_power)
	{
		buffer_PID.Output=chassis_max_power;
	}

	input_power = chassis_max_power - buffer_PID.Output; // Input power floating at maximum power
	
	chassis_max_power = input_power;
	for (uint8_t i = 0; i < 4; i++) // first get all the initial motor power and total motor power
	{
		initial_give_power[i] = M3508_speed_PID[i].Output * toque_coefficient * M3508[i].speed_rpm +
								k1 * M3508[i].speed_rpm * M3508[i].speed_rpm +
								k2 * M3508_speed_PID[i].Output* M3508_speed_PID[i].Output +
								constant;
								
		if (initial_give_power[i] < 0) // negative power not included (transitory)
			continue;
		initial_total_power += initial_give_power[i];
	}
	if (initial_total_power > chassis_max_power) // determine if larger than max power
	{
		float power_scale = chassis_max_power / initial_total_power;
		for (uint8_t i = 0; i < 4; i++)
		{
			scaled_give_power[i] = initial_give_power[i] * power_scale*0.81f; // get scaled power
			if (scaled_give_power[i] < 0)
			{
				continue;
			}

			float b = toque_coefficient * M3508[i].speed_rpm;
			float c = k2 * M3508[i].speed_rpm * M3508[i].speed_rpm - scaled_give_power[i] + constant;

			if (M3508_speed_PID[i].Output > 0) // Selection of the calculation formula according to the direction of the original moment
			{
				float temp = (-b + sqrt(b * b - 4 * k1 * c)) / (2 * k1);
				if (temp > 16000)
				{
					M3508_speed_PID[i].Output= 16000;
				}
				else
					M3508_speed_PID[i].Output = temp;
			}
			else
			{
				float temp = (-b - sqrt(b * b - 4 * k1 * c)) / (2 * k1);
				if (temp < -16000)
				{
					M3508_speed_PID[i].Output = -16000;
				}
				else
					M3508_speed_PID[i].Output = temp;
			}
		}
	}
}

//超电发送
void Super_power_ctrl(uint8_t Shift_Flag)//0 2充电1放电
{
	Can_Send_TO_Superpower(&hcan2,RobotStatust.chassis_power_limit,Shift_Flag,PowerHeatData.buffer_energy);

}

