#include "RS485.h"
#include "RC.h"
#include "Chassis_Task.h"
#include "judgement_info.h"

unsigned char *relative_ptr=0;

uint8_t RX_Data[2][36]; // 添加 volatile
uint8_t RS_485buffer = 0; // 添加 volatile
uint8_t TX_Data_t[16]={0};
uint8_t switch_AutoAim=0;//1开自瞄0不开
uint8_t xuruo=10;
uint8_t rfid=0;
extern RC_ctrl_t rc_ctrl;
extern UART_HandleTypeDef huart1;
extern Yaw_relative_union Yaw_motor;
extern int flag_reset;
extern ext_robot_status_t RobotStatust;
extern rc_mode_t Sentry_Mode;
extern ext_hurt_data_t HurtData;
extern ext_power_heat_data_t PowerHeatData;
extern ext_game_status_t GameState;
extern ext_shoot_data_t	ShootData;
extern ext_rfid_status_t RfidStatus;
extern ext_game_robot_HP_t GameRobotSurvivors;
static uint8_t unlocked = 1;   // 是否已解锁发射机构（1=已解锁，0=未解锁）


void RS485_Init(void)
{
	RS485_RX_EN()
}
void RS485_Send(uint8_t *TX_Data)
{
    TX_Data_t[0]=0x2b;
    TX_Data_t[15]=0xb2;
    
    if(Sentry_Mode==NAVI_MODE)
        TX_Data_t[1]=1;//0不过1过
    else
        TX_Data_t[1]=0;//0不过1过

    if(RobotStatust.robot_id==7)
        TX_Data_t[2]=1;//蓝方
    else if(RobotStatust.robot_id==107)
        TX_Data_t[2]=2;//红方
    else
        TX_Data_t[2]=3;//主控ID不是哨兵的
    
    TX_Data_t[3] = RobotStatust.shooter_barrel_heat_limit & 0xFF;        // 低字节 射击热量上限：260
    TX_Data_t[4] = (RobotStatust.shooter_barrel_heat_limit >> 8) & 0xFF; // 高字节
    TX_Data_t[5] = RobotStatust.shooter_barrel_cooling_value & 0xFF;//射击热量冷却速率：30/秒 
    TX_Data_t[6] = (RobotStatust.shooter_barrel_cooling_value >> 8) & 0xFF;
    TX_Data_t[7] = PowerHeatData.shooter_17mm_barrel_heat & 0xFF;//当前枪管热量
    TX_Data_t[8] = (PowerHeatData.shooter_17mm_barrel_heat >> 8) & 0xFF;//当前枪管热量
    if(GameState.game_progress==4)
        TX_Data_t[9]=1;//0不开火1开火   1
    else
        TX_Data_t[9]=0;//0不开火1开火   0
    relative_ptr=(unsigned char *)(&ShootData.initial_speed);
	TX_Data_t[10]=*relative_ptr;
	TX_Data_t[11]=*(relative_ptr+1);
	TX_Data_t[12]=*(relative_ptr+2);
	TX_Data_t[13]=*(relative_ptr+3);
    
    

// 死亡时重置解锁标志
    if (RobotStatust.current_HP == 0 && GameState.game_progress ==4)
    {
        unlocked = 0;   // 死亡后，需要重新进入补给区才能解锁
    }

// 在补给区内且尚未解锁时，执行解锁
    if (RfidStatus.Own_sup_non_ovlp_ex_rmul== 1 && unlocked == 0)
    {
        unlocked = 1;   // 标记为已解锁
    }

// 根据解锁标志控制发射机构
    if (unlocked == 1)
    {
        TX_Data_t[14] = 66;   // 解锁发射机构
    }
    else
    {
        TX_Data_t[14] = 12;   // 锁定发射机构
    }
        
    
//	HAL_UART_Transmit_DMA(&huart1, TX_Data,16u);//记得改这里，发送数组的大小
}

void RS485_IdleCallback(void)//在串口1的中断函数中调用，每进一次中断都会执行
{
    HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_SET);
    RS485_Send(TX_Data_t);
	RS485_TO_RC(RX_Data[RS_485buffer], &rc_ctrl);//解析485接收到的数据
	RS_485buffer = !RS_485buffer; // 切换缓冲区
//	HAL_UART_Receive_DMA(&huart1,RX_Data[RS_485buffer], 20u);//这个地方的大小要与下面实际使用到的大小保持一致，太大会出现0，太小数据不对
}


void RS485_TO_RC(volatile const uint8_t *RS485_buf, RC_ctrl_t *rc_ctrl)
{
    if (RS485_buf == NULL || rc_ctrl == NULL)
    {
        return;
    }
    else if(RS485_buf[0]==0x4a&&RS485_buf[19]==0xa4)
    {
        rc_ctrl->rc.ch[0]=(uint16_t)(RS485_buf[1] << 8 | RS485_buf[2]);
        rc_ctrl->rc.ch[1]=(uint16_t)(RS485_buf[3] << 8 | RS485_buf[4]);
        rc_ctrl->rc.ch[4]=(uint16_t)(RS485_buf[5] << 8 | RS485_buf[6]);
        rc_ctrl->rc.s[0]=RS485_buf[7];
        rc_ctrl->rc.s[1]=RS485_buf[8];
        
        Yaw_motor.Yaw_relativeArray[0]=RS485_buf[9];
        Yaw_motor.Yaw_relativeArray[1]=RS485_buf[10];
        Yaw_motor.Yaw_relativeArray[2]=RS485_buf[11];
        Yaw_motor.Yaw_relativeArray[3]=RS485_buf[12];
        
        if(RS485_buf[13]==11)
        {
            flag_reset=111;
        }
        else
            flag_reset=0;
        switch_AutoAim=RS485_buf[14];
        
        Yaw_motor.Yaw_relativeArray[4]=RS485_buf[15];
        Yaw_motor.Yaw_relativeArray[5]=RS485_buf[16];
        Yaw_motor.Yaw_relativeArray[6]=RS485_buf[17];
        Yaw_motor.Yaw_relativeArray[7]=RS485_buf[18];
    }
    
	RC_MODE_CONTROL();//获取遥控器模式
}
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart->Instance == USART1)
//    {
//        // 在这里切换回接收模式
//        HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_RESET);
//    }
//}

