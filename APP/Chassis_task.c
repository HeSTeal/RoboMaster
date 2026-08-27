#include "Chassis_task.h"
#include "controller.h"
#include "RS485.h"
#include "user_lib.h"
#include "cmsis_os.h"
#include "Motor.h"
#include "RC.h"
#include "can.h"
#include "power_control.h"
#include "Ramp_Control.h"
#include "stdbool.h"
#include "NAVI_Task.h"
#include "usart.h"
#include <stdio.h>
#include "judgement_info.h"

PID_t M3508_speed_PID[4]={0};
PID_t chassis_Follow_PID,chassis_Follow_angle,chassis_Follow_speed;
chassis_struct AGV_chassis;
Yaw_relative_union Yaw_motor;
RampGen_t VzRamp;
uint8_t flag_tt=1;
float speed_max=3.04f;//单位是 m/s，以3508额定转速计算，跑满转速车的速度为4.3m/s，速度分解后为3.04
TickType_t tickCount = 0;
float currentTime =0;
float AGV_w=0;//底盘旋转角速度，度/s
float AGV_speed[4]={0};
float mid360_vx=0,mid360_vy=0,mid360_vw=0;//这两个的作用相当于AGV_chassis.x_speed,AGV_chassis.y_speed
int gyro_time=0;
float gyro_t=0;
float Relative=0;
float a=0;
float follow_angle = 0; 
extern RC_ctrl_t rc_ctrl;
extern motor_measure_t M3508[4];
extern rc_mode_t Sentry_Mode;
extern NAVI_RX_union NAVI_Data;
extern damiao_struct Yaw_4310_motor;
extern ext_sentry_info_t SentryInfo;//0x020D
extern ext_game_status_t GameState;//0x0001 GameState.stage_remain_time

extern Super_power_t Super_power;//超电
extern float aa;
extern uint8_t falg_t;
extern uint8_t zimiao_mode;
extern ext_robot_status_t RobotStatust;
extern ext_projectile_allowance_t ProjectileAllowance;//0x0208

float YAW_offset[4]={-2.17f,2.55f,0.97f,-0.59f};

void Chassis_PID_Init(void)
{
    //              PID结构体      PID最大输出 积分限幅  误差死区   P   I   D  微分先行系数A  微分先行系数B  输出滤波  微分滤波  最小二乘法样本数（微分）
    PID_Init_t(&M3508_speed_PID[0],   16000   ,  1000  ,     0    ,10 , 0 , 0 ,      0       ,     0       ,     0    ,    0    ,          0         ,0xff);
    PID_Init_t(&M3508_speed_PID[1],   16000   ,  1000  ,     0    ,10 , 0 , 0 ,      0       ,     0       ,     0    ,    0    ,          0         ,0xff);
    PID_Init_t(&M3508_speed_PID[2],   16000   ,  1000  ,     0    ,10 , 0 , 0 ,      0       ,     0       ,     0    ,    0    ,          0         ,0xff);
    PID_Init_t(&M3508_speed_PID[3],   16000   ,  1000  ,     0    ,10 , 0 , 0 ,      0       ,     0       ,     0    ,    0    ,          0         ,0xff);
//3  10
//    PID_Init_t(&chassis_Follow_PID,      3   ,    20  ,     0    ,10 , 0 , 0 ,      0       ,     0       ,     0    ,    0    ,          0         ,0xff);
    PID_Init_t(&chassis_Follow_PID,     12    ,    22  ,     0    ,25 , 0 , 0 ,      0       ,     0       ,     0    ,    0    ,          0         ,0xff);
    PID_Init_t(&chassis_Follow_angle,    30   ,    5   ,     0    , 0 , 0 , 0 ,      0       ,     0       ,     0    ,    0    ,          0         ,0xff);
    PID_Init_t(&chassis_Follow_speed,    4    ,    20  ,     0    , 0 , 0 , 0 ,      0       ,     0       ,     0    ,    0    ,          0         ,0xff);

    Power_Limit_Init();

    RampSetScale(&VzRamp,250);
}

int flag_t=0;
uint16_t CNT_count=0;
int16_t Temp[2][4]={0};
void CAN_Detect(motor_measure_t *M3508,motor_measure_t *GM6020,int16_t temp[2][4])
{
//只要有一个电机数据没有更新且持续时间>200，就会触发100
    if(M3508[0].given_current==temp[0][0]||M3508[1].given_current==temp[0][1]||M3508[2].given_current==temp[0][2]||
        M3508[3].given_current==temp[0][3])//有一个电机数据没有更新
    {
        
        CNT_count++;
        if(CNT_count>400)
        {
            flag_t=100;//100有问题
        }
    }
//只有4个电机数据都更新才会为触发111
    if(M3508[0].given_current!=temp[0][0]&&M3508[1].given_current!=temp[0][1]&&M3508[2].given_current!=temp[0][2]&&
        M3508[3].given_current!=temp[0][3])//电机数据都更新
    {
        flag_t=111;//111没问题
        CNT_count=0;//没问题就要归0，否则CNT_count会及缓慢递增
    }
    for(int i=0;i<4;i++)
    {
        temp[0][i]=M3508[i].given_current;
        //temp[1][i]=GM6020[i].given_current;
    }
}

void AGV_speed_calc(chassis_struct *AGV, float *out_speed)
{
    out_speed[0] = (1.414f*( AGV->vx + AGV->vy) + AGV->vw*WHEEL_DISTANCE )*GEAR_RATIO*60/(2*PI*WHEEL_RADIUS);//0
    out_speed[1] = (1.414f*(-AGV->vx + AGV->vy) + AGV->vw*WHEEL_DISTANCE )*GEAR_RATIO*60/(2*PI*WHEEL_RADIUS);//1
    out_speed[2] = (1.414f*(-AGV->vx - AGV->vy) + AGV->vw*WHEEL_DISTANCE )*GEAR_RATIO*60/(2*PI*WHEEL_RADIUS);//2
    out_speed[3] = (1.414f*( AGV->vx - AGV->vy) + AGV->vw*WHEEL_DISTANCE )*GEAR_RATIO*60/(2*PI*WHEEL_RADIUS);//3
    
    float max_out_speed=0;
    for(int i=0;i<4;i++)
    {
        if(fabs(out_speed[i])>max_out_speed)
        {
            max_out_speed=fabs(out_speed[i]);//寻找最大输出转速
        }
    }
    if(max_out_speed>MAX_speed_rpm)
    {
        float scale=MAX_speed_rpm/max_out_speed;//计算缩放因子
        for(int i=0;i<4;i++)
            out_speed[i]*=scale;
    }

    for(int i=0;i<4;i++)
    {
        PID_Calculate(&M3508_speed_PID[i], M3508[i].speed_rpm , out_speed[i] );
    }
}


/* 
 * 该函数通过计算四个轮子的线速度之和，进而计算出底盘的角速度。
 * 
 * @return fp32 底盘的角速度（单位：弧度/秒）
 */
float calculate_chassis_angular_speed(void)
{
    // 定义一个数组，用于存储四个轮子的线速度
    float wheel_speeds[4];
    
    // 遍历四个轮子
    for (int i = 0; i < 4; i++)
    {
        // 将RPM转换为线速度（m/s）
        float rpm = M3508[i].speed_rpm;
        float wheel_speed = (rpm / GEAR_RATIO) * (2 * PI * WHEEL_RADIUS) / 60.0f;
        wheel_speeds[i] = wheel_speed;
    }
    
    // 计算四个轮子的线速度之和
    float sum_v = wheel_speeds[0] + wheel_speeds[1] + wheel_speeds[2] + wheel_speeds[3];
    
    // 计算底盘的角速度（rad/s）
    float omega = sum_v / (4 * WHEEL_DISTANCE) / (360.0f / 8192.0f);
    
    return omega/180.0f*3.1415926f;
}
float Float_sign(float input, float offset)
{
    static uint32_t input_sign;
    static uint32_t offset_num;
    static uint32_t output;
    input_sign = *(uint32_t*)&input;
    input_sign = input_sign&0x80000000;
    offset_num = *(uint32_t*)&offset;
    offset_num = input_sign&0x7fffffff;
    output = offset_num | input_sign;
    return *(float*)&output;
}
float get_relative_pos(float ref_angle, float target_angle, float max_scope)
{
    float deviation=target_angle-ref_angle;
		if(deviation>max_scope/2){
      deviation = deviation - max_scope;
    }
		else if(deviation<-max_scope/2){
			deviation = deviation + max_scope;	
    }
    return deviation;
}
float dead_zone(float Value, float minValue, float maxValue)
{
    if (Value < maxValue && Value > minValue)
    {
        Value = 0.0f;
    }

    return Value;
}

void NAVI_Controller(void)
{
    if(GameState.stage_remain_time<420&&GameState.stage_remain_time>=415)
    {
        AGV_chassis.vx=0.5f;
        AGV_chassis.vy=0.0f;
    }
    else
    {
        AGV_chassis.vx=0.0f;
        AGV_chassis.vy=0.0f;
         AGV_chassis.vw=9.0f;
    }
//    if(GameState.stage_remain_time<418&&GameState.stage_remain_time>=416)
//    {
//        AGV_chassis.vx=1.5f;
//        AGV_chassis.vy=0.0f;
//    }
//    if(GameState.stage_remain_time<416&&GameState.stage_remain_time>=412)
//    {
//        AGV_chassis.vx=0.0f;
//        AGV_chassis.vy=1.5f;
//    }
//    if(GameState.stage_remain_time<412&&GameState.stage_remain_time>=410)
//    {
//        AGV_chassis.vx=-1.5f;
//        AGV_chassis.vy=0.0f;
//    }
//    if(GameState.stage_remain_time<410&&GameState.stage_remain_time>=408)
//    {
//        AGV_chassis.vx=-1.5f;
//        AGV_chassis.vy=-1.5f;
//    }
//    if(GameState.stage_remain_time<408&&GameState.stage_remain_time>=404)
//    {
//        AGV_chassis.vx=0.0f;
//        AGV_chassis.vy=-1.5f;
//    }
}

//void NAVI_Controller(void)
//{
//    // 已过去的时间（从 420 开始递减，则过去时间 = 420 - 当前剩余时间）
//    float elapsed = 420.0f - GameState.stage_remain_time;
//    if (elapsed < 0.0f) elapsed = 0.0f;

//    // 周期长度 16 秒，计算在当前周期内的相位 (0~16)
//    float phase = fmodf(elapsed, 16.0f);

//    // 根据相位所在区间设置速度（与原始时间区间完全对应）
//    if (phase >= 0.0f && phase < 2.0f) {    
//        AGV_chassis.vx = 0.3f;  AGV_chassis.vy = 0.0f;
//    } else if (phase >= 2.0f && phase < 4.0f) {
//        AGV_chassis.vx = 0.0f;  AGV_chassis.vy = -0.3f;
//    } else if (phase >= 4.0f && phase < 8.0f) {
//        AGV_chassis.vx = 0.0f;  AGV_chassis.vy = 1.5f;
//    } else if (phase >= 8.0f && phase < 10.0f) {
//        AGV_chassis.vx = -1.5f; AGV_chassis.vy = 0.0f;
//    } else if (phase >= 10.0f && phase < 12.0f) {
//        AGV_chassis.vx = -1.5f; AGV_chassis.vy = -1.5f;
//    } else {  // phase 12.0 ~ 16.0
//        AGV_chassis.vx = 0.0f;  AGV_chassis.vy = -1.5f;
//    }
//}
void No_Force_mode_t(void)
{
    AGV_chassis.vx=AGV_chassis.vy=AGV_chassis.vw=0;
    CAN2_Send(0,0,0,0);
}

void GIMBAL_SEPARATE_Chassis_t(void)
{
    CAN_Detect(M3508,0,Temp);

    AGV_chassis.vx=AGV_chassis.x_speed*cos(Yaw_motor.YAW.Yaw_relative*PI/180.0f)-AGV_chassis.y_speed*sin(Yaw_motor.YAW.Yaw_relative*PI/180.0f);
    AGV_chassis.vy=AGV_chassis.x_speed*sin(Yaw_motor.YAW.Yaw_relative*PI/180.0f)+AGV_chassis.y_speed*cos(Yaw_motor.YAW.Yaw_relative*PI/180.0f);
    AGV_chassis.vw=0;

    AGV_speed_calc(&AGV_chassis,AGV_speed);
    chassis_power_control();
    
    if(flag_t==111)
    {
        CAN2_Send(M3508_speed_PID[0].Output,M3508_speed_PID[1].Output,M3508_speed_PID[2].Output,M3508_speed_PID[3].Output); 
    }
    else
    {
    
    }
}

void Chassis_FOLLOW_GIMBAL_t(void)
{
    CAN_Detect(M3508,0,Temp);

    RampResetCounter(&VzRamp);

//    PID_Calculate(&chassis_Follow_PID,follow_angle*PI/180.0f,0.0f);//单环   
//    AGV_chassis.vx=AGV_chassis.x_speed*cos(Yaw_motor.YAW.Yaw_relative*PI/180.0f)-AGV_chassis.y_speed*sin(Yaw_motor.YAW.Yaw_relative*PI/180.0f);
//    AGV_chassis.vy=AGV_chassis.x_speed*sin(Yaw_motor.YAW.Yaw_relative*PI/180.0f)+AGV_chassis.y_speed*cos(Yaw_motor.YAW.Yaw_relative*PI/180.0f);
    
    PID_Calculate(&chassis_Follow_PID,Relative*PI/180.0f,0);//单环   
    AGV_chassis.vx=AGV_chassis.x_speed*cos(Relative*PI/180.0f)-AGV_chassis.y_speed*sin(Relative*PI/180.0f);
    AGV_chassis.vy=AGV_chassis.x_speed*sin(Relative*PI/180.0f)+AGV_chassis.y_speed*cos(Relative*PI/180.0f);
    AGV_chassis.vw=-chassis_Follow_PID.Output;
    
    AGV_speed_calc(&AGV_chassis,AGV_speed);
    chassis_power_control();

    if(flag_t==111)
    {
        CAN2_Send(M3508_speed_PID[0].Output,M3508_speed_PID[1].Output,M3508_speed_PID[2].Output,M3508_speed_PID[3].Output); 
    }
    else
    {
    
    }
}

void Gyro_MODE_t(void)
{
    CAN_Detect(M3508,0,Temp);

    static bool last_up_t = false;      // 记录上拨的上一状态
    static bool last_down_t = false;    // 记录下拨的上一状态

    bool current_up_t   = (rc_ctrl.rc.ch[4] < -500);   // 上拨判定
    bool current_down_t = (rc_ctrl.rc.ch[4] > 500);    // 下拨判定（阈值可调）

    // 上拨上升沿触发：增加
    if (current_up_t && !last_up_t) 
    {
        a -= 0.8f;
    }

    // 下拨上升沿触发：减小
    if (current_down_t && !last_down_t) 
    {
        a += 0.8f;
    }
    if(a>16)
    {
        a=16;
    }
    if(a<-16)
    {
        a-=-16;
    }
    // 更新状态
    last_up_t   = current_up_t;
    last_down_t = current_down_t;
    //逆时针转往左偏给正值，往右偏给负值
    AGV_chassis.vx=AGV_chassis.x_speed*cos(Yaw_motor.YAW.Yaw_relative*PI/180.0f)-AGV_chassis.y_speed*sin(Yaw_motor.YAW.Yaw_relative*PI/180.0f);
    AGV_chassis.vy=AGV_chassis.x_speed*sin(Yaw_motor.YAW.Yaw_relative*PI/180.0f)+AGV_chassis.y_speed*cos(Yaw_motor.YAW.Yaw_relative*PI/180.0f);

//    AGV_chassis.vx=AGV_chassis.x_speed*cos(Yaw_motor.YAW.Yaw_relative*PI/180.0f-11.0f*PI/180.0f)-AGV_chassis.y_speed*sin(Yaw_motor.YAW.Yaw_relative*PI/180.0f-11.0f*PI/180.0f);
//    AGV_chassis.vy=AGV_chassis.x_speed*sin(Yaw_motor.YAW.Yaw_relative*PI/180.0f-11.0f*PI/180.0f)+AGV_chassis.y_speed*cos(Yaw_motor.YAW.Yaw_relative*PI/180.0f-11.0f*PI/180.0f);
//    AGV_chassis.vw=rc_ctrl.rc.ch[4]/11.0f;
    AGV_chassis.vw=a;
            
    AGV_speed_calc(&AGV_chassis,AGV_speed);
    chassis_power_control();

    if(flag_t==111)
    {
        CAN2_Send(M3508_speed_PID[0].Output,M3508_speed_PID[1].Output,M3508_speed_PID[2].Output,M3508_speed_PID[3].Output); 
    }
    else
    {
    
    }
}
/************************ 单向脉冲变速小陀螺参数 ************************/

/*
 * 小陀螺方向：
 *  1.0f  正转
 * -1.0f  反转
 *
 * 如果你发现实际旋转方向不是你想要的，就把 1.0f 改成 -1.0f。
 */
#define GAME_GYRO_DIR              1.0f

#define GAME_GYRO_MIN_W            5.0f
#define GAME_GYRO_MAX_W            7.6f

#define GAME_GYRO_ACCEL_UP         18.0f
#define GAME_GYRO_ACCEL_DOWN       32.0f
/*
 * 导航速度判零死区：
 * 浮点数不要直接和 0 比较。
 */
#define NAVI_SPEED_EPS             0.02f


static float Game_Gyro_Pulse_Update(bool enable)
{
    /*
     * 速度表设计思路：
     * 低速段时间长一点，用来降低离心力；
     * 高速段时间短一点，用来保持防打效果；
     * 高速之后快速降速，打断弹丸稳定贴外侧的状态。
     */
    static const float target_w_table[] =
    {
        5.2f,
        7.2f,
        5.0f,
        7.5f,
        5.3f,
        7.0f,
        5.1f,
        7.6f,
        5.4f,
        7.3f
    };

    static const uint16_t hold_time_ms_table[] =
    {
        260,
        160,
        340,
        160,
        300,
        150,
        360,
        160,
        280,
        150
    };
    
    static bool active = false;
    static uint8_t index = 0;

    static TickType_t last_tick = 0;
    static TickType_t target_tick = 0;

    static float vw_cmd = 0.0f;

    TickType_t now_tick = xTaskGetTickCount();

    if (!enable)
    {
        active = false;
        index = 0;
        last_tick = now_tick;
        target_tick = now_tick;
        vw_cmd = 0.0f;
        return 0.0f;
    }

    if (!active)
    {
        active = true;
        index = 0;
        last_tick = now_tick;
        target_tick = now_tick;

        /*
         * 第一次进入直接给非零速度，
         * 不从 0 慢慢爬升，避免露静止靶。
         */
        vw_cmd = GAME_GYRO_DIR * GAME_GYRO_MIN_W;
        return vw_cmd;
    }

    float dt = (float)(now_tick - last_tick) / (float)configTICK_RATE_HZ;
    last_tick = now_tick;

    /*
     * 防止任务阻塞或调试暂停导致 dt 过大，
     * 从而让速度指令突然跳变。
     */
    if (dt <= 0.0f || dt > 0.05f)
    {
        dt = 1.0f / (float)configTICK_RATE_HZ;
    }

    uint32_t elapsed_ms =
        (uint32_t)(((uint64_t)(now_tick - target_tick) * 1000U) /
                   configTICK_RATE_HZ);

    if (elapsed_ms >= hold_time_ms_table[index])
    {
        index++;

        if (index >= sizeof(target_w_table) / sizeof(target_w_table[0]))
        {
            index = 0;
        }

        target_tick = now_tick;
    }

    float target_vw = GAME_GYRO_DIR * target_w_table[index];
    float error = target_vw - vw_cmd;

    /*
     * 关键：
     * 升速慢，降速快。
     *
     * 正转时：target_vw < vw_cmd 表示降速；
     * 反转时：target_vw > vw_cmd 表示降速，因为负数绝对值变小。
     */
    float accel;

    if (GAME_GYRO_DIR > 0.0f)
    {
        if (target_vw < vw_cmd)
        {
            accel = GAME_GYRO_ACCEL_DOWN;
        }
        else
        {
            accel = GAME_GYRO_ACCEL_UP;
        }
    }
    else
    {
        if (target_vw > vw_cmd)
        {
            accel = GAME_GYRO_ACCEL_DOWN;
        }
        else
        {
            accel = GAME_GYRO_ACCEL_UP;
        }
    }

    float max_step = accel * dt;

    if (error > max_step)
    {
        vw_cmd += max_step;
    }
    else if (error < -max_step)
    {
        vw_cmd -= max_step;
    }
    else
    {
        vw_cmd = target_vw;
    }

    /*
     * 兜底限幅：
     * 不允许掉到最低小陀螺速度以下；
     * 不允许超过最高速度。
     */
    if (GAME_GYRO_DIR > 0.0f)
    {
        if (vw_cmd < GAME_GYRO_MIN_W)
        {
            vw_cmd = GAME_GYRO_MIN_W;
        }

        if (vw_cmd > GAME_GYRO_MAX_W)
        {
            vw_cmd = GAME_GYRO_MAX_W;
        }
    }
    else
    {
        if (vw_cmd > -GAME_GYRO_MIN_W)
        {
            vw_cmd = -GAME_GYRO_MIN_W;
        }

        if (vw_cmd < -GAME_GYRO_MAX_W)
        {
            vw_cmd = -GAME_GYRO_MAX_W;
        }
    }

    return vw_cmd;
}

void AutoAim_MODE_t(void)
{
    CAN_Detect(M3508,0,Temp);
    
    static bool last_up = false;
    static bool gyro_enable = false;

    bool current_up = (rc_ctrl.rc.ch[4] < -500);
    RampResetCounter(&VzRamp);
/*************************处理目标+控制器+输出到驱动器********************************************/        
    AGV_chassis.vx=AGV_chassis.x_speed*cos(Yaw_motor.YAW.Yaw_relative*PI/180.0f)-AGV_chassis.y_speed*sin(Yaw_motor.YAW.Yaw_relative*PI/180.0f);
    AGV_chassis.vy=AGV_chassis.x_speed*sin(Yaw_motor.YAW.Yaw_relative*PI/180.0f)+AGV_chassis.y_speed*cos(Yaw_motor.YAW.Yaw_relative*PI/180.0f);
    
    if (current_up && !last_up)
    {
        gyro_enable = !gyro_enable;

        if (!gyro_enable)
        {
            Game_Gyro_Pulse_Update(false);
        }
    }

    last_up = current_up;

    if (gyro_enable)
    {
        AGV_chassis.vw = Game_Gyro_Pulse_Update(true);
    }
    else
    {
        AGV_chassis.vw = 0.0f;
        Game_Gyro_Pulse_Update(false);
    }
    
    AGV_speed_calc(&AGV_chassis,AGV_speed);
    chassis_power_control();

    if(flag_t==111)
    {
        CAN2_Send(M3508_speed_PID[0].Output,M3508_speed_PID[1].Output,M3508_speed_PID[2].Output,M3508_speed_PID[3].Output); 
    }
    else
    {
    
    }
}

void NAVI_MODE_t(void)
{
    CAN_Detect(M3508,0,Temp);
    if(NAVI_Data.NAVI_RX_Data.V_X!=0||NAVI_Data.NAVI_RX_Data.V_Y!=0||NAVI_Data.NAVI_RX_Data.V_Z!=0)
    {
        mid360_vx=NAVI_Data.NAVI_RX_Data.V_X;
        mid360_vy=NAVI_Data.NAVI_RX_Data.V_Y;
        mid360_vw=NAVI_Data.NAVI_RX_Data.V_Z;
        if(NAVI_Data.NAVI_RX_Data.V_X==0&&NAVI_Data.NAVI_RX_Data.V_Y!=0)
        {
            AGV_chassis.vx=0;
            AGV_chassis.vy=mid360_vy;//不需要加cos sin进行转换，视觉那边已经转了
            AGV_chassis.vw=mid360_vw;
        }
        else if(NAVI_Data.NAVI_RX_Data.V_X!=0&&NAVI_Data.NAVI_RX_Data.V_Y==0)
        {
            AGV_chassis.vx=mid360_vx;
            AGV_chassis.vy=0;
            AGV_chassis.vw=mid360_vw;
        }
        else
        {
            AGV_chassis.vx=mid360_vx;
            AGV_chassis.vy=mid360_vy;
            AGV_chassis.vw=mid360_vw;
        }
    }
    else//导航那边数据为0了
    {
        AGV_chassis.vx=0;
        AGV_chassis.vy=0;
        AGV_chassis.vw=0.0f;
    }
    AGV_speed_calc(&AGV_chassis,AGV_speed);
    chassis_power_control();

    if(flag_t==111)
    {
        CAN2_Send(M3508_speed_PID[0].Output,M3508_speed_PID[1].Output,M3508_speed_PID[2].Output,M3508_speed_PID[3].Output); 
    }
    else
    {
    
    }
}

void GAME_MODE_t(void)
{
    CAN_Detect(M3508,0,Temp);
    if(Super_power.volt>13500&&SentryInfo.sentry_posture!=3&&flag_tt==1)
    {
        Super_power_ctrl(1);//放电
    }
    else 
    {
        flag_tt=2;
        Super_power_ctrl(2);
        if(Super_power.volt>=18500)
            flag_tt=1;
    }
    if(GameState.game_progress==4)
    {
        if(zimiao_mode!=0&&RobotStatust.current_HP>100&&ProjectileAllowance.projectile_allowance_17mm>20)
        {
            AGV_chassis.vx=AGV_chassis.vy=0;
            AGV_chassis.vw = Game_Gyro_Pulse_Update(true);
        }
        else
        {
            if(NAVI_Data.NAVI_RX_Data.V_X!=0||NAVI_Data.NAVI_RX_Data.V_Y!=0||NAVI_Data.NAVI_RX_Data.V_Z!=0)
            {
                mid360_vx=NAVI_Data.NAVI_RX_Data.V_X;
                mid360_vy=NAVI_Data.NAVI_RX_Data.V_Y;
                mid360_vw=NAVI_Data.NAVI_RX_Data.V_Z;
                if(NAVI_Data.NAVI_RX_Data.V_X==0&&NAVI_Data.NAVI_RX_Data.V_Y!=0)
                {
                    AGV_chassis.vx=0;
                    AGV_chassis.vy=mid360_vy;//不需要加cos sin进行转换，视觉那边已经转了
                    AGV_chassis.vw=mid360_vw;
                }
                else if(NAVI_Data.NAVI_RX_Data.V_X!=0&&NAVI_Data.NAVI_RX_Data.V_Y==0)
                {
                    AGV_chassis.vx=mid360_vx;
                    AGV_chassis.vy=0;
                    AGV_chassis.vw=mid360_vw;
                }
                else
                {
                    AGV_chassis.vx=mid360_vx;
                    AGV_chassis.vy=mid360_vy;
                    AGV_chassis.vw=mid360_vw;
                }
            }
            else//导航那边数据为0了
            {
                AGV_chassis.vx=0;
                AGV_chassis.vy=0;
                AGV_chassis.vw = Game_Gyro_Pulse_Update(true);
               
            }
        }
    
        AGV_speed_calc(&AGV_chassis,AGV_speed);
        chassis_power_control();
        
        if(flag_t==111)
        {
            CAN2_Send(M3508_speed_PID[0].Output,M3508_speed_PID[1].Output,M3508_speed_PID[2].Output,M3508_speed_PID[3].Output); 
        }
        else
        {
        
        }
    }
}

void Chassis_type(rc_mode_t Sentry_mode)
{
    switch(Sentry_mode)
    {
        case No_Force_mode :            No_Force_mode_t(); break;
        case GIMBAL_SEPARATE_Chassis :  GIMBAL_SEPARATE_Chassis_t(); break;
        case Chassis_FOLLOW_GIMBAL :    Chassis_FOLLOW_GIMBAL_t(); break;
        case Gyro_MODE :                Gyro_MODE_t(); break;
        case AutoAim_MODE :             AutoAim_MODE_t(); break;
        case NAVI_MODE :                NAVI_MODE_t(); break;
        case GAME_MODE :                GAME_MODE_t(); break;
    }
}

void Chassis_movment(void)
{
    rc_ctrl.rc.ch[0]=dead_zone( rc_ctrl.rc.ch[0] , -20 , 20 );//死区
    rc_ctrl.rc.ch[1]=dead_zone( rc_ctrl.rc.ch[1] , -20 , 20 );
    rc_ctrl.rc.ch[4]=dead_zone( rc_ctrl.rc.ch[4] , -40 , 40 );
    
    AGV_chassis.x_speed=rc_ctrl.rc.ch[1]/660.0f*speed_max;//归一化,不能除以660，当整数处理
    AGV_chassis.y_speed=-rc_ctrl.rc.ch[0]/660.0f*speed_max;
    AGV_w=calculate_chassis_angular_speed();//底盘转速
    
    tickCount=xTaskGetTickCount();//系统时间
	currentTime=(float)tickCount / (float)configTICK_RATE_HZ;
    
    Relative=-(360.0f/6.28f)*get_relative_pos(Yaw_4310_motor.position,NAVI_yaw,6.28);//导航要求的
    Yaw_motor.YAW.Yaw_relative=-(360.0f/6.28f)*get_relative_pos(Yaw_4310_motor.position,Controller_offset,6.28);
    
    if(Yaw_4310_motor.position>=-2.9f&&Yaw_4310_motor.position<=-1.28f)
            follow_angle=-(360.0f/6.28f)*get_relative_pos(Yaw_4310_motor.position,YAW_offset[0],6.28);
    if(Yaw_4310_motor.position<-2.9f||Yaw_4310_motor.position>=1.82f)
            follow_angle=-(360.0f/6.28f)*get_relative_pos(Yaw_4310_motor.position,YAW_offset[1],6.28);
    if(Yaw_4310_motor.position>=0.23f&&Yaw_4310_motor.position<1.82f)
            follow_angle=-(360.0f/6.28f)*get_relative_pos(Yaw_4310_motor.position,YAW_offset[2],6.28);
    if(Yaw_4310_motor.position>=-1.28f&&Yaw_4310_motor.position<0.23f)
            follow_angle=-(360.0f/6.28f)*get_relative_pos(Yaw_4310_motor.position,YAW_offset[3],6.28);

    Chassis_type(Sentry_Mode);
    RC_MODE_CONTROL();//保证复位
}
