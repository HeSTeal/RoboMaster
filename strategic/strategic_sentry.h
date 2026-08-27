#ifndef __STRATEGIC_SENTRY_H__
#define __STRATEGIC_SENTRY_H__

#include "main.h"



/****************************开始标志*********************/
#define UI_SOF 0xA5
/****************************CMD_ID数据********************/
#define UI_CMD_Robo_Exchange 0x0301    
/****************************内容ID数据********************/
#define UI_Data_ID_Del 0x100 
#define UI_Data_ID_Draw1 0x101
#define UI_Data_ID_Draw2 0x102
#define UI_Data_ID_Draw5 0x103
#define UI_Data_ID_Draw7 0x104
#define UI_Data_ID_DrawChar 0x110
#define SENTRY_Self_Service_Decision   0x0120
#define radar_Self_Service_Decision   0x0121
/****************************红方机器人ID********************/
#define UI_Data_RobotID_RHero 1         
#define UI_Data_RobotID_REngineer 2
#define UI_Data_RobotID_RStandard1 3
#define UI_Data_RobotID_RStandard2 4
#define UI_Data_RobotID_RStandard3 5
#define UI_Data_RobotID_RAerial 6
#define UI_Data_RobotID_RSentry 7
#define UI_Data_RobotID_RRadar 9
/****************************蓝方机器人ID********************/
#define UI_Data_RobotID_BHero 101
#define UI_Data_RobotID_BEngineer 102
#define UI_Data_RobotID_BStandard1 103
#define UI_Data_RobotID_BStandard2 104
#define UI_Data_RobotID_BStandard3 105
#define UI_Data_RobotID_BAerial 106
#define UI_Data_RobotID_BSentry 107
#define UI_Data_RobotID_BRadar 109
/**************************红方操作手ID************************/
#define UI_Data_CilentID_RHero 0x0101
#define UI_Data_CilentID_REngineer 0x0102
#define UI_Data_CilentID_RStandard1 0x0103
#define UI_Data_CilentID_RStandard2 0x0104
#define UI_Data_CilentID_RStandard3 0x0105
#define UI_Data_CilentID_RAerial 0x0106
/***************************蓝方操作手ID***********************/
#define UI_Data_CilentID_BHero 0x0165
#define UI_Data_CilentID_BEngineer 0x0166
#define UI_Data_CilentID_BStandard1 0x0167
#define UI_Data_CilentID_BStandard2 0x0168
#define UI_Data_CilentID_BStandard3 0x0169
#define UI_Data_CilentID_BAerial 0x016A
#define ID_JUDGEMENT     0x8080



typedef unsigned char Uint8_t;
typedef unsigned char U8;


typedef struct
{
   uint16_t bullet;              
   uint8_t far_bullet_times;    
   uint8_t bllod_times; 
   uint8_t recover; 
   uint8_t nowrecover; 
   uint16_t recover_coin; 
	
   uint8_t disengage; 
   uint16_t convertibility_bullet; 

} sentry_sturct;        

typedef struct
{
  uint8_t 	base;              						// bit 0：己方基地增益点
  uint8_t 	own_annular;                  // bit 1：己方环形高地增益点
  uint8_t 	adverse_annular;              // bit 2：对方环形高地增益点
	uint8_t	 	own_r3withb3;                 // bit 3：己方 R3/B3 梯形高地增益点
	uint8_t	 	adverse_r3withb3;             // bit 4：对方 R3/B3 梯形高地增益点
	uint8_t	 	own_r4withb4;                 // bit 5：己方 R4/B4 梯形高地增益点
	uint8_t	 	adverse_r4withb4;             // bit 6：对方 R4/B4 梯形高地增益点
	uint8_t		energy_mechanism;             // bit 7：己方能量机关激活点
	uint8_t		own_flying_slope1;            // bit 8：己方飞坡增益点（靠近己方一侧飞坡前）
	uint8_t		own_flying_slope2;            // bit 9：己方飞坡增益点（靠近己方一侧飞坡后）
	uint8_t		adverse_flying_slope1;        // bit 10：对方飞坡增益点（靠近对方一侧飞坡前）
	uint8_t		adverse_flying_slope2;        // bit 11：对方飞坡增益点（靠近对方一侧飞坡后）
	uint8_t		own_outpost;                  // bit 12：己方前哨站增益点
	uint8_t		bloodgain_point;              // bit 13：己方补血点（检测到任一均视为激活）
	uint8_t		own_sentry_patrol;            // bit 14：己方哨兵巡逻区
	uint8_t		adverse_sentry_patrol;        // bit 15：对方哨兵巡逻区
	uint8_t		own_resources_island;         // bit 16：己方大资源岛增益点
	uint8_t		adverse_resources_island;     // bit 17：对方大资源岛增益点
	uint8_t		own_exchange;                 // bit 18：己方兑换区
	uint8_t		reverse19;           
	uint8_t		reverse20;
	uint8_t		reverse21;
	uint8_t		reverse22;
	uint8_t		central_gain_point;			  // bit 23：中心增益点（仅 RMUL 适用）


} rfid_sturct;                            // bit 20-31：保留

typedef __packed struct
{
   uint8_t SOF;                    //起始字节,固定0xA5
   uint16_t Data_Length;           //帧数据长度
   uint8_t Seq;                    //包序号
   uint8_t CRC8;                   //CRC8校验值
   uint16_t CMD_ID;                //命令ID
} UI_Packhead;             //帧头

typedef struct
{
   uint16_t Data_ID;               //内容ID
   uint16_t Sender_ID;             //发送者ID
   uint16_t Receiver_ID;           //接收者ID
} UI_Data_Operate;         //操作定义帧


typedef struct//？？是否加内存对齐
{
    uint32_t revive :1;                 //是否确认复活（读条）
    uint32_t immediate_revive :1;       //花钱买活
    uint32_t shot_count :11;            //在补血点可兑换的允许发弹量（但规则中说是前哨站增益点也可以）
    uint32_t shot_count_cishu :4;       //远程兑换发弹量的次数
    uint32_t remote_blood_count :4;     //远程兑换血量的次数
    uint32_t sentry_posture :3;         //当前姿态
    uint32_t enable_Energy_Device:1;    //哨兵确认能量机处于激活状态
    uint32_t reversed:7;
} sentry_cmd_t;

typedef struct
{
uint8_t radar_cmd;
} radar_cmd_t;

typedef __packed struct
{
    uint8_t *shoot_flag;
}connection_gimbal;

/************************************************************************************/
void SENTRy_Service_Decision(uint8_t resurgence,uint8_t immediater_esurgence,uint16_t bullet,uint8_t request_times,uint8_t blood);
unsigned char Get_CRC8_Check_Sum_UI(unsigned char *pchMessage,unsigned int dwLength,unsigned char ucCRC8);
uint16_t Get_CRC16_Check_Sum_UI(uint8_t *pchMessage,uint32_t dwLength,uint16_t wCRC);

#endif
