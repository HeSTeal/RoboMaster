#ifndef __RC_H_
#define	__RC_H_

#include "main.h"

#define RC_CH_VALUE_OFFSET      ((uint16_t)1024)

typedef enum
{
    No_Force_mode = 0,//无力模式
	GIMBAL_SEPARATE_Chassis = 1,//云台分离
	Chassis_FOLLOW_GIMBAL,//底盘跟随
	Gyro_MODE,//小陀螺
	AutoAim_MODE,//自瞄
	NAVI_MODE,//导航
    GAME_MODE,//比赛模式
} rc_mode_t;



#define RC_SW_UP ((uint16_t)1)
#define RC_SW_MID ((uint16_t)3)
#define RC_SW_DOWN ((uint16_t)2)
#define switch_is_down(s) (s == RC_SW_DOWN)
#define switch_is_mid(s) (s == RC_SW_MID)
#define switch_is_up(s) (s == RC_SW_UP)


void RC_MODE_CONTROL(void);
void sbus_to_rc(volatile const uint8_t *sbus_buf);
#endif


