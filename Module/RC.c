#include "RC.h"
#include "RS485.h"


RC_ctrl_t rc_ctrl;
rc_mode_t Sentry_Mode = No_Force_mode;//哨兵模式，初始化为双下，在串口1中断中获取USART1_IRQHandler
int flag_reset=0;
int flag_switch=0;
void RC_MODE_CONTROL(void)
{
//    if (rc_ctrl.rc.ch[0] == -660 && rc_ctrl.rc.ch[1] == -660 && 
//        rc_ctrl.rc.ch[2] == 660 && rc_ctrl.rc.ch[3] == -660) // 拨杆内八软件强制复位
//    {
//        __disable_irq();        // 禁用所有中断（等效于__set_FAULTMASK(1)）
//        HAL_NVIC_SystemReset(); // 系统复位
//    }
    if (rc_ctrl.rc.ch[0] == -660 && rc_ctrl.rc.ch[1] == -660)
    {
        flag_switch=3;
    }
    if (flag_reset==111) //为了节省字节，用这个表示强制
    {
        __disable_irq();        // 禁用所有中断（等效于__set_FAULTMASK(1)）
        HAL_NVIC_SystemReset(); // 系统复位
    }
//    if (rc_ctrl.rc.s[0] == RC_SW_MID)//右中
//    {
//        if (rc_ctrl.rc.s[1] == RC_SW_DOWN)//左下
//            Sentry_Mode = GIMBAL_SEPARATE_Chassis;//云台分离
//        else if (rc_ctrl.rc.s[1] == RC_SW_MID)//左中
//            Sentry_Mode = Chassis_FOLLOW_GIMBAL;//底盘跟随
//        else if (rc_ctrl.rc.s[1] == RC_SW_UP)//左上
//            Sentry_Mode = Gyro_MODE;//小陀螺
//    }
//    else if (rc_ctrl.rc.s[0] == RC_SW_UP)//右上
//    {
//        if (rc_ctrl.rc.s[1] == RC_SW_DOWN)//左下
//            Sentry_Mode = AutoAim_MODE;//自瞄
//        else if (rc_ctrl.rc.s[1] == RC_SW_MID)//左中
//            Sentry_Mode = NAVI_MODE;//导航
//        else if(rc_ctrl.rc.s[1] == RC_SW_UP )//左上
//            Sentry_Mode = GAME_MODE;//比赛
//    }
//    else
//        Sentry_Mode = No_Force_mode;//双下
    
}

/**
  * @brief          遥控器协议解析
  * @param[in]      sbus_buf: 原生数据指针
  * @param[out]     rc_ctrl: 遥控器数据指
  * @retval         none
  */
void sbus_to_rc(volatile const uint8_t *sbus_buf)
{
   if (sbus_buf == NULL)
    {
        return;
    }

    rc_ctrl.rc.ch[0] = (sbus_buf[0] | (sbus_buf[1] << 8)) & 0x07ff;        
    rc_ctrl.rc.ch[1] = ((sbus_buf[1] >> 3) | (sbus_buf[2] << 5)) & 0x07ff; 
    rc_ctrl.rc.ch[2] = ((sbus_buf[2] >> 6) | (sbus_buf[3] << 2) | (sbus_buf[4] << 10)) & 0x07ff;  
    rc_ctrl.rc.ch[3] = ((sbus_buf[4] >> 1) | (sbus_buf[5] << 7)) & 0x07ff; 
    rc_ctrl.rc.s[0] = ((sbus_buf[5] >> 4) & 0x0003);                  
    rc_ctrl.rc.s[1] = ((sbus_buf[5] >> 4) & 0x000C) >> 2;                
    rc_ctrl.rc.ch[4] = sbus_buf[16] | (sbus_buf[17] << 8);                
          
    rc_ctrl.rc.ch[0] -= RC_CH_VALUE_OFFSET;
    rc_ctrl.rc.ch[1] -= RC_CH_VALUE_OFFSET;
    rc_ctrl.rc.ch[2] -= RC_CH_VALUE_OFFSET;
    rc_ctrl.rc.ch[3] -= RC_CH_VALUE_OFFSET;
    rc_ctrl.rc.ch[4] -= RC_CH_VALUE_OFFSET;
    
//    HAL_IWDG_Refresh(&hiwdg);
}





             
