#ifndef __CHASSIS_TASK_H
#define __CHASSIS_TASK_H

#define MAX_speed_rpm       8800
#define GEAR_RATIO          15.765f    // 电机减速比
#define WHEEL_RADIUS        0.0725f    // 轮子半径（米）
#define WHEEL_DISTANCE      0.295f     // 轮子到底盘中心的距离（米） 

#define NAVI_yaw -2.65   //导航模式的yaw固定值
#define Controller_offset -1.90   //控制零点
typedef struct
{
	float vx,vy,vw;// m/s m/s rad/s 
	float x_speed,y_speed,z_speed;

}chassis_struct;

typedef __packed struct
{
    float Yaw_relative;//-180  +180
    float YAW_speed;//-30 +30
} Yaw_relative_struct;

typedef __packed union
{
    Yaw_relative_struct YAW;
    unsigned char Yaw_relativeArray[8];
} Yaw_relative_union;
void Chassis_PID_Init(void);
void Chassis_movment(void);
#endif

