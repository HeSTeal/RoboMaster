#include "NAVI_Task.h"
#include "usbd_cdc_if.h"
#include "ins_task.h"
#include "rng.h"
#include "protocol.h"
#include "ins_task.h"
#include "RGB.h"
#include "bsp_dwt.h"
#include "Motor.h"
#include "RC.h"

uint8_t NAVI_Data_t[USB_Length];
NAVI_RX_union NAVI_Data;
Judge_union Judge_date;
short last_random=0;
uint16_t SAVE_CNT=0; 

extern uint8_t BUF[35];
extern INS_t INS;
extern RGB_enum RGB_color;

extern ext_robot_status_t RobotStatust;
extern ext_game_status_t GameState;
extern ext_projectile_allowance_t ProjectileAllowance;
extern ext_buff_t Buff;
extern ext_event_data_t EventData;
extern ext_hurt_data_t HurtData;
extern ext_game_robot_HP_t GameRobotSurvivors;
extern ext_map_command_t MapCommand;                
extern rc_mode_t Sentry_Mode;
extern ext_referee_warning_t RefereeWarning;         
void NAVI_RX(void)
{
    if(verify_crc8_check_sum( &NAVI_Data_t[1], 21)!=NULL )//通过CRC校验    大小是除去帧头帧尾的
    {
        memcpy(NAVI_Data.NAVIRXArray,NAVI_Data_t,USB_Length);//转移数据
      
        if(NAVI_Data.NAVI_RX_Data.random==last_random)//防止导航那边的数据是错误的
        {
            SAVE_CNT++;
            if(SAVE_CNT>600)
            {
                NAVI_Data.NAVI_RX_Data.V_X=0;
                NAVI_Data.NAVI_RX_Data.V_Y=0;
                NAVI_Data.NAVI_RX_Data.V_Z=0;
                for(uint8_t i=0;i<35;i++)
                {
                    BUF[i]=0;
                }
                for(uint8_t i=0;i<USB_Length;i++)
                {
                    NAVI_Data_t[i]=0;
                }
                
//                __disable_irq();    // 关闭所有中断
//                NVIC_SystemReset(); // 软件复位
            }
        }
        else
        {
            SAVE_CNT=0;
            if(NAVI_Data.NAVI_RX_Data.V_X!=0||NAVI_Data.NAVI_RX_Data.V_Y!=0||NAVI_Data.NAVI_RX_Data.V_Z!=0)
            {
               RGB_color=RGB_WHITE;
            }
            else if(NAVI_Data.NAVI_RX_Data.V_X==0&&NAVI_Data.NAVI_RX_Data.V_Y==0&&NAVI_Data.NAVI_RX_Data.random==0)
            {
                RGB_color=RGB_RED;
            }
        } 
        last_random=NAVI_Data.NAVI_RX_Data.random;
    }
    else
    {
        NAVI_Data.NAVI_RX_Data.V_X=0;
        NAVI_Data.NAVI_RX_Data.V_Y=0;
        NAVI_Data.NAVI_RX_Data.V_Z=0;
        RGB_color=RGB_RED;
//        NAVI_Data.NAVI_RX_Data.chassis_type=0;
    }
}
uint16_t adc=0;
int asd=0;
float error_t=0,last_HP_t=0;
float fanzhi,fangyu_time=0;
int hurt_reason=127;
uint8_t fangyu=0,fangyu_t=0;
extern damiao_struct Yaw_4310_motor;
void NAVI_TX(void)
{
    unsigned char crc = 0;
    Judge_date.Judege_data_t.head=0x7C;
    Judge_date.Judege_data_t.tail=0xC7;
    Judge_date.Judege_data_t.sentry_HP=RobotStatust.current_HP;//当前血量
    Judge_date.Judege_data_t.mode=GameState.game_type;//比赛类型
    Judge_date.Judege_data_t.race_time=GameState.stage_remain_time;//比赛剩余时间
    Judge_date.Judege_data_t.convertibility_bullet=Sentry_Mode;//没找到
    Judge_date.Judege_data_t.bullet_remaining_num_17mm=ProjectileAllowance.projectile_allowance_17mm;//允许发弹量，UL初始为750，正常不断减，UL不能买弹
    Judge_date.Judege_data_t.defence_buff=GameRobotSurvivors.ally_base_HP;//防御增益(换成我方基地血量)
    Judge_date.Judege_data_t.myoutpost=EventData.Outpost_BUFF;//前哨战占领情况
    Judge_date.Judege_data_t.observeoutpost=0;//今年没有，要走雷达链路
    Judge_date.Judege_data_t.start_race=GameState.game_progress;//比赛进程
    
    if(RobotStatust.robot_id==107)
    {
        if(MapCommand.target_position_x>1.55f&&MapCommand.target_position_x<=3.35f
            &&MapCommand.target_position_y>=6.7f&&MapCommand.target_position_y<=8.5f)
            Judge_date.Judege_data_t.center_state=2;
        else
            Judge_date.Judege_data_t.center_state=1;//中心增益点(换成云台手标志位冲家用)
        
    }
    
    if(RobotStatust.robot_id==7)
    {
        if(MapCommand.target_position_x>24.5f&&MapCommand.target_position_x<=26.5f
            &&MapCommand.target_position_y>=6.6f&&MapCommand.target_position_y<=8.5f)
            Judge_date.Judege_data_t.center_state=2;
        else
            Judge_date.Judege_data_t.center_state=1;//中心增益点(换成云台手标志位冲家用)
        
    }
    
    
   error_t=last_HP_t-RobotStatust.current_HP;
    last_HP_t=RobotStatust.current_HP;
    
    if(error_t>5&&HurtData.HP_deduction_reason==0)//不受伤时间差一直增加
    {
        fangyu_t=2;//检测是否进入标志
        fangyu_time=DWT_GetTimeline_s();
    }

    if(DWT_GetTimeline_s() - fangyu_time<3.0f)//一旦受伤时间差一直更新，一直小于1
        fangyu=3;
    else
        fangyu=1;
    
    if(HurtData.HP_deduction_reason==0&&error_t>0)
    {
        fanzhi = DWT_GetTimeline_s();
    }
    
    if(DWT_GetTimeline_s() - fanzhi<4.0f)
    {
        hurt_reason=0;
    }
    else
        hurt_reason=127;
    
//    if(HurtData.HP_deduction_reason==0&&error_t>0)
//    {
//        fanzhi = DWT_GetTimeline_s();
//    }
//    
//    if(DWT_GetTimeline_s() - fanzhi<4.0f)
//    {
//        Judge_date.Judege_data_t.hurt_reason=0;
//    }
//    else
//        Judge_date.Judege_data_t.hurt_reason=127;
//    Judge_date.Judege_data_t.hero_HP=GameRobotSurvivors.hero_HP;
//    Judge_date.Judege_data_t.balance3_HP=GameRobotSurvivors.balance_1_HP;
    Judge_date.Judege_data_t.random=Get_RandomNumber_Range(1000,6000); 
	crc = get_crc8_check_sum(&Judge_date.Iudge_arr[1],18, 0xff);//大小为去掉帧头帧尾CRC的大小
	Judge_date.Judege_data_t.crc = crc;
    
    CDC_Transmit_FS(&Judge_date.Iudge_arr[0],21);//大小为结构体大小
    
}

