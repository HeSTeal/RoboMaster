#include "strategic_sentry.h"
#include "judgement_info.h"
#include "cmsis_os.h"
#include "bsp_dwt.h"
#include "Chassis_Task.h"
#include "NAVI_Task.h"
extern UART_HandleTypeDef huart5;
/*******************需要用到的裁判系统指令结构体*********************/
//比赛剩余时间
extern ext_game_status_t GameState;//0x0001 GameState.stage_remain_time
//哨兵血量 基地血量  前哨站血量
extern ext_game_robot_HP_t GameRobotSurvivors;//0x0003 GameRobotSurvivors.sentry_HP GameRobotSurvivors.Base_HP  GameRobotSurvivors.Outpost_HP
//前哨站 堡垒 地形跨越（中央高地）补给区 能量机关 
extern ext_event_data_t EventData;//0x0101
//本机器人当前热量冷却值 功率上限
extern ext_robot_status_t RobotStatust;//0x0201
//缓冲能量 17mm发射机构的热量
extern ext_power_heat_data_t PowerHeatData;//0x0202
//机器人位置 x y
extern ext_robot_pos_t RobotPos;//0x0203
//冷却增益 防御增益 攻击增益 剩余能量
extern ext_buff_t Buff;//0x0204
//血量受到弹丸攻击扣血（结合自身血量使用）
extern ext_hurt_data_t HurtData;//0x0206
//弹丸射速 初速度（自瞄可能会用）
extern ext_shoot_data_t	ShootData;//0x0207
//自身17mm允许发弹量 金币数 堡垒增益点
extern ext_projectile_allowance_t ProjectileAllowance;//0x0208
//场地交互
extern ext_rfid_status_t RfidStatus;//0x0209
//敌方机器人到达易伤情况
extern ext_radar_mark_data_t RadarMarkData;//0x020C
//哨兵自主决策（以及底下决策函数的命令结构体）
extern ext_sentry_info_t SentryInfo;//0x020D
//冲家用
extern ext_map_command_t MapCommand;//0x303
extern ext_robot_status_t RobotStatust;
/********************************************************************/
//        允许复活              立即复活                   申请次数            血量
uint8_t sentryresurgence=1,sentryimmediater_esurgence=0,sentryrequest_times=0,sentryblood=0;
uint32_t Robot_ID,Cilent_ID;
unsigned char UI_Seq;//包序号
uint16_t sentrybullet=0;//剩余弹量
sentry_cmd_t imageData;//子命令0x0120的结构体
extern chassis_struct AGV_chassis;
extern float currentTime;
extern int flag_switch;
extern NAVI_RX_union NAVI_Data;
extern int hurt_reason;
/*哨兵自主决策,注：兑换金币不足时此命令组无效，裁判系统将收发下一组数据
远程购买弹量时request_times需增加一次
远程兑换要求脱战 机器人在存活状态下连续 6 秒未发射弹丸且未被扣血。
远程兑换成功后，允许发弹量将在 6 秒后生效。若确定远程兑换血量 6 秒内，机器人战亡或被罚下，则远程兑换血量无效，且金币不会返
还。
resurgence***************************是否复活指令——只可从零到一
immediater_esurgence*****************是否立即复活——只可从零到一
bullet*******************************购弹量此值的变化需要单调递增，否则视为不合法。最大值为2047单场
request_times************************远程购弹次数单次递增
blood********************************
在哨兵发送该子命令时，服务器将按照从相对低位到相对高位
的原则依次处理这些指令，直至全部成功或不能处理为止。
示例：若队伍金币数为 0，此时哨兵战亡，“是否确认复活”的
值为 1，“是否确认兑换立即复活”的值为 1，“确认兑换的允
许发弹量值”为 100。（假定之前哨兵未兑换过允许发弹量）由
于此时队伍金币数不足以使哨兵兑换立即复活，则服务器将会
忽视后续指令，等待哨兵发送的下一组指令。
*/
uint8_t falg_t=0,fuhuo=0;
extern uint8_t zimiao_mode;
extern float error_t;
float fuhuo_time=0;
uint16_t buxiefangyu=0,count_zimao=0;


uint8_t switch_t=0,switch_tt=0;
uint16_t t_17mm_t,t_17mm_error,t_17mm_t_last;
float switch_time=0,switch_time_t=0;

void panduan(void)
{
    t_17mm_error=t_17mm_t_last-ProjectileAllowance.projectile_allowance_17mm;
    t_17mm_t_last=ProjectileAllowance.projectile_allowance_17mm;
    
    if(zimiao_mode!=0)//自瞄看到人一直进，一直刷新时间，就会一直小于五秒
    {
        switch_time_t=DWT_GetTimeline_s();
    }
    if(DWT_GetTimeline_s() - switch_time_t<5.0f)//没看到人小于五秒，切进攻
    {
        switch_tt=0;
    }
    else
        switch_tt=2;
        

    if(t_17mm_error>0)//只要不发弹时间就会只记录一次，发弹就会一直记录
    {
        switch_time = DWT_GetTimeline_s();
    }
    if(DWT_GetTimeline_s() - switch_time<5.0f)
    {
        switch_t=0;
    }
    else
        switch_t=2;
}

//读条复活，哨兵是否需要立即复活？？？？
void revive_t(uint16_t HP)
{
    if(HP==0)
        imageData.revive=1;//死了，给裁判端发1请求复活
    else
        imageData.revive=0;//没死，就不请求复活，否则会影响姿态切换
}
//补给区买弹
void blood_requst(uint16_t supply_point,uint32_t blood_count,uint8_t time)
{    
    if(supply_point==1&&blood_count<=200&&ProjectileAllowance.remaining_gold_coin>=400&&!time)//占领补给区且发弹量小于等于200且剩余金币大于等于400
        imageData.shot_count=150;//就买150发弹
    if(SentryInfo.sentry_allowed_ammunition_count==150)
        time=1;
}
//前哨站买弹
void blood_requst_t(uint16_t supply_point,uint32_t blood_count,uint8_t time2)
{
    if(supply_point==1&&blood_count<=200&&ProjectileAllowance.remaining_gold_coin>=400&&!time2)//占领补给区且发弹量小于等于200且剩余金币大于等于400
        imageData.shot_count=100;//就买150发弹
    if(SentryInfo.sentry_allowed_ammunition_count==100)
        time2=1;
}
//远程复活，远程买弹
void remote_revive(uint32_t blood,uint16_t HP_t,uint8_t time1)
{
    if(HP_t==0&&ProjectileAllowance.remaining_gold_coin>400)
        imageData.immediate_revive=1;
    else
        imageData.immediate_revive=0;
    if(blood<180&&!time1)
    {
        imageData.shot_count_cishu=1;//150买100发
        imageData.shot_count=50;
    }
        
    if(SentryInfo.sentry_redemptions_count==1)
        time1=1;
}
extern uint8_t fangyu;
uint8_t asdf;
//场地围挡在红方补给站附近的交点为坐标原点，沿场地长边向蓝方为 X 轴正方向，沿场地短边
//向红方停机坪为 Y 轴正方向。
void SENTRy_Service_Decision(uint8_t resurgence,uint8_t immediater_esurgence,uint16_t bullet,uint8_t request_times,uint8_t blood)		
{  
    UI_Packhead framehead;                        //帧头和命令ID结构体
    UI_Data_Operate datahead;                   //命令ID的内容，数据段结构体
    uint16_t frametail=0xFFFF;                        //CRC16校验值，帧尾
    
    unsigned char *framepoint;                      //读写指针
    
    framepoint=(unsigned char *)&framehead;
/********************开始进行数据包的配置**************************/    
    framehead.SOF=UI_SOF;
    framehead.Data_Length=10;//2+2+2+4
    framehead.Seq=UI_Seq;
    framehead.CRC8=Get_CRC8_Check_Sum_UI(framepoint,4,0xFF);
/****************以上完成帧头的配置****************************/
    framehead.CMD_ID=UI_CMD_Robo_Exchange;                   //命令ID  0x0301
/****************以上完成cmd_id的配置******************************/   
    datahead.Data_ID=SENTRY_Self_Service_Decision;//子内容ID   0x0120
    datahead.Sender_ID=Robot_ID;//发送方ID   直接由裁判系统读到
    datahead.Receiver_ID=0x8080;//接收方ID    接受方是服务器 
/*****************子内容中的内容数据段*************************/ 
    revive_t(RobotStatust.current_HP);
    
    panduan();
     
    //机器人处于红方半场                                                        瞄到人          剩余3-5min中时                                                  强化剩余时间大于0         
    if(GameState.game_progress==4&&RobotPos.x>1.0f&&RobotPos.x<9.2f&&RobotPos.y>3.0f&&RobotPos.y<11.5f&&zimiao_mode==2&&GameState.stage_remain_time<300&&GameState.stage_remain_time>180&&SentryInfo.attack_enhanced_remaining>0)
        imageData.sentry_posture=4;
    else if(GameState.game_progress==4&&RobotPos.x>1.0f&&RobotPos.x<9.2f&&RobotPos.y>3.0f&&RobotPos.y<11.5f&&hurt_reason==0&&SentryInfo.defense_enhanced_remaining>0&&GameState.stage_remain_time<180&&GameState.stage_remain_time>60)
        imageData.sentry_posture=5;
    else if(GameState.game_progress==4)
    {
        if(zimiao_mode!=0||(RobotStatust.current_HP<=350&&RobotStatust.current_HP>250))
            imageData.sentry_posture=2;
        if(RobotStatust.current_HP>100&&RobotStatust.current_HP<=250)
        {
            if(switch_t==0||switch_tt==0)//五秒内看到人或发弹
            {
                imageData.sentry_posture=1;
            }
            else
                imageData.sentry_posture=2;
        }
        if(RobotStatust.current_HP<=100)   
            imageData.sentry_posture=3;
    }
    else
    {
        imageData.sentry_posture=3;
    }


    
/*******************以上完成0x0301命令ID的配置*************************/   
    framepoint=(unsigned char *)&framehead;
    frametail=Get_CRC16_Check_Sum_UI(framepoint,sizeof(framehead),frametail);
    framepoint=(unsigned char *)&datahead;
    frametail=Get_CRC16_Check_Sum_UI(framepoint,sizeof(datahead),frametail);//分开计算也是一样的，frametail虽然分开计算但是值也在变，可以串起来
    framepoint=(unsigned char *)&imageData;
    frametail=Get_CRC16_Check_Sum_UI(framepoint,sizeof(imageData),frametail);             //CRC16校验   //CRC16校验值计算（部分）  
/********************以上完成帧尾的配置*****************************/
    HAL_UART_Transmit(&huart5, (uint8_t *)&framehead, sizeof(framehead), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)&datahead, sizeof(datahead), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)&imageData, sizeof(imageData), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart5, (uint8_t *)&frametail, sizeof(frametail), HAL_MAX_DELAY);
///*************************发给服务器帧头********************************/  
//    while (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_TC) == RESET) {}// 等待上一次发送彻底完成
//    __HAL_UART_CLEAR_FLAG(&huart5, UART_FLAG_TC); // 清除标志，准备下一次检测
//    HAL_UART_Transmit_DMA(&huart5, (uint8_t *)&framehead, sizeof(framehead));
///*************************发给服务器命令ID的内容********************************/  
//    while (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_TC) == RESET) {}
//    __HAL_UART_CLEAR_FLAG(&huart5, UART_FLAG_TC);
//    HAL_UART_Transmit_DMA(&huart5, (uint8_t *)&datahead, sizeof(datahead));
///**************************发给服务器数据段内容**************************/   
//    while (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_TC) == RESET) {}
//    __HAL_UART_CLEAR_FLAG(&huart5, UART_FLAG_TC);
//    HAL_UART_Transmit_DMA(&huart5, (uint8_t *)&imageData, sizeof(imageData));
///**************************发给服务器帧尾**************************/     
//    while (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_TC) == RESET) {}
//    __HAL_UART_CLEAR_FLAG(&huart5, UART_FLAG_TC);
//    HAL_UART_Transmit_DMA(&huart5, (uint8_t *)&frametail, sizeof(frametail));
        
    UI_Seq++;                                                         //包序号+1

}    
///*************************发给服务器帧头********************************/  
//    framepoint=(unsigned char *)&framehead;
////    for(int i=0;i<sizeof(framehead);i++)
////    {
////        HAL_UART_Transmit_DMA(&huart5,framepoint, 1);
////        framepoint++;
////    }
//    HAL_UART_Transmit_DMA(&huart5, (uint8_t *)&framehead, sizeof(framehead));
//    osDelay(30);
///*************************发给服务器命令ID的内容********************************/  
//    framepoint=(unsigned char *)&datahead;
////    for(int i=0;i<sizeof(datahead);i++)
////    {      
////        HAL_UART_Transmit_DMA(&huart5,framepoint, 1);																
////        framepoint++;
////    }    
//    HAL_UART_Transmit_DMA(&huart5, (uint8_t *)&datahead, sizeof(datahead));
//    osDelay(30);
///**************************发给服务器数据段内容**************************/   
//    framepoint=(unsigned char *)&imageData;
////    for(int i=0;i<sizeof(imageData);i++)
////    {
////        HAL_UART_Transmit_DMA(&huart5,framepoint, 1);																	//发送操作数据
////        framepoint++;             
////    }     
//    HAL_UART_Transmit_DMA(&huart5, (uint8_t *)&imageData, sizeof(imageData));
//    osDelay(30);
///**************************发给服务器帧尾**************************/      
//    framepoint=(unsigned char *)&frametail;
////    for(int i=0;i<sizeof(frametail);i++)
////    {
////        HAL_UART_Transmit_DMA(&huart5,framepoint, 1);																	 //发送CRC16校验值
////        framepoint++;                                                 
////    }
//    HAL_UART_Transmit_DMA(&huart5, (uint8_t *)&frametail, sizeof(frametail));
//    osDelay(20);
//   

//    blood_requst(RfidStatus.Own_sup_ovlp_ex,ProjectileAllowance.projectile_allowance_17mm,0);
//    blood_requst_t(RfidStatus.Own_outpost_pt,ProjectileAllowance.projectile_allowance_17mm,0);
//    remote_revive(ProjectileAllowance.projectile_allowance_17mm,RobotStatust.current_HP,0);
//    if(GameState.stage_remain_time<180)
//        imageData.sentry_posture=2;
//    else if(zimiao_mode==2)
//        imageData.sentry_posture=1;
//    else
//        imageData.sentry_posture=3;
    
//第二场    
//    if(GameState.stage_remain_time<360)
//        imageData.sentry_posture=2;
    
//    if(falg_t==3)
//    {
//        imageData.sentry_posture=3;
//    }
//    if(falg_t==2)
//    {
//        imageData.sentry_posture=2;
//    }
//    if(falg_t==1)
//    {
//        imageData.sentry_posture=1;
//    }

connection_gimbal chassis_and_gimbal;
uint8_t change_time;
uint8_t set_flag[3];//是否到达对面就家的标志位，可以由导航那边发过来，也可以电控根据自己的场地位置进行判断，后面要改到结构体里
void strategic_sentry(connection_gimbal up_connnection_down)
{
////先上坎到对面家里，然后停在那里开防御模式进行自瞄,自瞄到就转为进攻模式，没有就为防御
////进攻模式是否开小陀螺自保，可以根据自己防御增益来判断（前三十秒可以不开小陀螺，有50%的地形跨越增益，在底盘中加Buff.defence_buff==0判断）
//    if(set_flag[0]==1)
//    {
//        if(change_time>5&&*up_connnection_down.shoot_flag==0)//冷却时间大于5S并且没有瞄到
//            SentryInfo.sentry_posture=2;//切换为防御姿态（这里的小陀螺在底盘导航模式下结合导航速度进行判断）
//        else if(*up_connnection_down.shoot_flag==1)//自瞄已经瞄准
//        {   
//            if(change_time>5&&SentryInfo.sentry_posture!=1)//且冷却时间大于5S
//                SentryInfo.sentry_posture=1;//切换为进攻姿态
//        }
//    }
//    else if(GameRobotSurvivors.outpost_HP==0&&set_flag[1]==0)//转移到前哨站，可以对前哨站进行复活，
//    {
//        //给导航发送一个数据
//        if(RfidStatus.status_union.bits.your_outpost==1)//到达一个位置后导航再给一个值
//        {
//            set_flag[1]=1;//结束导航，转为作战
//        }
//    }
//    else if((MapCommand.target_position_x>18&&MapCommand.target_position_x<19)&&
//     (MapCommand.target_position_y>4&&MapCommand.target_position_y<6))
//    {
//        //给导航发送一个值，去对面家里
//        
//    }
}
//void UC_test(uint8_t AutoAim,ext_sentry_info_t Sentry_Posture)
//{
//    if(AutoAim==1)//自瞄瞄到敌人给下板发1，否则为0
//    {
//        Sentry_Posture.sentry_posture=1;//0x20D中姿态值切换为进攻姿态
//        RobotStatust.chassis_power_limit=75;//功率上限降为1/2
//        RobotStatust.shooter_barrel_cooling_value=120;//获得3倍热量冷却增益
//        Buff.cooling_buff=90;
//        Buff.vulnerability_buff=-25;//获得25%易伤
//        AGV_chassis.vw=2.5f;//进攻时开启小陀螺
//    }
//    else if(HurtData.HP_deduction_reason==0)//当装甲板受到小弹丸击打时为0
//    {
//        Sentry_Posture.sentry_posture=2;//切换为防御姿态
//        RobotStatust.chassis_power_limit=75;//功率上限降为1/2
//        RobotStatust.shooter_barrel_cooling_value=10;//热量冷却速率降为1/3
//        Buff.defence_buff=50;//获得50%的防御增益
//        AGV_chassis.vw=3.0f;//防御时开启小陀螺
//    }
//    else
//    {
//        Sentry_Posture.sentry_posture=3;//以上两种情况都不符合则为移动姿态
//        RobotStatust.chassis_power_limit=150;//功率上限提升为1.5倍
//        Buff.vulnerability_buff=-25;//获得25%易伤
//        RobotStatust.shooter_barrel_cooling_value=10;//热量冷却速率降为1/3
//    }
//}
    
extern uint32_t start5;
extern double Strategic_time;

extern void StartStrategic(void *argument)  			 //任务函数
{
	while(1)
	{
//        DWT_GetDeltaT64(&start5);
		Robot_ID=RobotStatust.robot_id;
		Cilent_ID=ID_JUDGEMENT;	
        
		SENTRy_Service_Decision(sentryresurgence,sentryimmediater_esurgence,sentrybullet,sentryrequest_times,sentryblood);
//        Strategic_time=DWT_GetDeltaT64(&start5);
		vTaskDelay(100);
	}   
}


/*****************************************************CRC8校验值计算**********************************************/
const unsigned char CRC8_INIT_UI = 0xff; 
const unsigned char CRC8_TAB_UI[256] = 
{ 
0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41, 
0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e, 0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc, 
0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0, 0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62, 
0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d, 0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff, 
0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5, 0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07, 
0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58, 0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a, 
0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6, 0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24, 
0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b, 0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9, 
0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f, 0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd, 
0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92, 0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50, 
0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c, 0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee, 
0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1, 0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73, 
0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49, 0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b, 
0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4, 0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16, 
0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a, 0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8, 
0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7, 0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35, 
};
unsigned char Get_CRC8_Check_Sum_UI(unsigned char *pchMessage,unsigned int dwLength,unsigned char ucCRC8) 
{ 
unsigned char ucIndex; 
while (dwLength--) 
{ 
ucIndex = ucCRC8^(*pchMessage++); 
ucCRC8 = CRC8_TAB_UI[ucIndex]; 
} 
return(ucCRC8); 
}

uint16_t CRC_INIT_UI = 0xffff; 
const uint16_t wCRC_Table_UI[256] = 
{ 
0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 
0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7, 
0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e, 
0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876, 
0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd, 
0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 
0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c, 
0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974, 
0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb, 
0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3, 
0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72, 
0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 
0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1, 
0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 
0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 
0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff, 
0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036, 
0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e, 
0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5, 
0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd, 
0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134, 
0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c, 
0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3, 
0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb, 
0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 
0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a, 
0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 
0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9, 
0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 
0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};
/* 
** Descriptions: CRC16 checksum function 
** Input: Data to check,Stream length, initialized checksum 
** Output: CRC checksum 
*/ 
uint16_t Get_CRC16_Check_Sum_UI(uint8_t *pchMessage,uint32_t dwLength,uint16_t wCRC) 
{ 
Uint8_t chData; 
if (pchMessage == NULL) 
{ 
return 0xFFFF; 
} 
while(dwLength--) 
{ 
chData = *pchMessage++;
(wCRC) = ((uint16_t)(wCRC) >> 8) ^ wCRC_Table_UI[((uint16_t)(wCRC) ^ (uint16_t)(chData)) & 
0x00ff]; 
} 
return wCRC; 
}

