#ifndef __JUDGEMENT_INFO_H__
#define __JUDGEMENT_INFO_H__

#include "main.h"

//帧头详细定义
#define    LEN_HEADER    5        //帧头长
#define    LEN_CMDID     2        //命令码长度
#define    LEN_TAIL      2	      //帧尾CRC16

/* 自定义帧头 */
typedef __packed struct
{
	uint8_t  SOF;
	uint16_t DataLength;
	uint8_t  Seq;
	uint8_t  CRC8;
} xFrameHeader;

/********************************************* 命令码 ID 和常规链路数据说明 ***************************************************************
    命令码		数据段长度	说明							        频率						        发送方/接收方				                            所属数据链路
    ID: 0x0001  Byte: 11    比赛状态数据       				        固定1Hz      				        服务器→全体机器人			                            常规链路
    ID: 0x0002  Byte:  1    比赛结果数据         			        比赛结束后发送      		        服务器→全体机器人			                            常规链路
    ID: 0x0003  Byte: 16	机器人血量数据(26赛季只有己方机器人)    固定3Hz						        服务器→全体机器人 （其实就是己方机器人）	            常规链路        

    ID: 0x0101  Byte:  4    场地事件数据   					        固定1Hz						        服务器→己方全体机器人		                            常规链路       	
    ID: 0X0104  Byte:  3    裁判警告数据					        己方判罚/判负时触发发送，	        服务器→被判罚方全体机器人	                            常规链路															其余时间以 1Hz 频率发送
    ID: 0x0105  Byte:  3    飞镖发射相关数据				        固定1Hz						        服务器→己方全体机器人		                            常规链路

    ID: 0x0201  Byte: 13    机器人性能体系数据        		        固定10Hz					        主控模块→己方全体机器人	                            常规链路
    ID: 0x0202  Byte: 14    实时底盘缓冲能量和射击热量数据          固定10Hz       				        主控模块→对应机器人	                                常规链路
    ID: 0x0203  Byte: 16    机器人位置数据           		        固定1Hz						        主控模块→对应机器人		                            常规链路
    ID: 0x0204  Byte:  8    机器人增益和底盘能量数据                固定3Hz						        服务器→对应机器人			                            常规链路
    ID: 0x0206  Byte:  1    伤害状态数据           			        伤害发生后发送				        主控模块→对应机器人		                            常规链路
    ID: 0x0207  Byte:  7    实时射击数据           			        子弹发射后发送				        主控模块→对应机器人						            常规链路
    ID: 0x0208  Byte:  6    允许发弹量						        固定10Hz					        服务器→己方英雄、步兵、哨兵、空中机器人	            常规链路
    ID: 0x0209  Byte:  5    机器人 RFID 状态，				        固定3Hz						        服务器→己方装有 RFID模块的机器人			            常规链路
    ID: 0x020A	Byte:  6	飞镖选手端指令数据		    	        固定3Hz						        服务器→己方飞镖机器人		                            常规链路
    ID: 0x020B	Byte: 40	地面机器人位置数据				        固定1Hz						        服务器→己方哨兵机器人		                            常规链路
    ID: 0x020C	Byte:  2	雷达标记进度数据				        固定1Hz 					        服务器→己方雷达机器人		                            常规链路
    ID: 0x020D	Byte:  6	哨兵自主决策信息同步			        固定1Hz						        服务器→己方哨兵机器人		                            常规链路
    ID: 0x020E	Byte:  1	雷达自主决策信息同步			        固定1Hz						        服务器→己方雷达机器人		                            常规链路
                                                                                                                                                        
    ID: 0x0301  Byte:127    机器人间交互数据				        发送方触发发送,频率上限为 30Hz	    -											            常规链路
    ID: 0x0302  Byte: 30    自定义控制器交互数据接口		        发送方触发发送，频率上限为 30Hz	    自定义控制器→选手端图传连接的机器人			        图传链路
    ID: 0x0303	Byte: 15	选手端小地图交互数据			        选手端触发发送					    选手端点击→服务器→发送方选择的己方机器人	            常规链路
    ID: 0x0304	Byte: 12	键鼠遥控数据					        固定30Hz						    选手端→选手端图传连接的机器人				            图传链路
    ID: 0x0305	Byte：24	选手端小地图接收雷达数据		        频率上限为 5Hz					    雷达→服务器→己方所有选手端					        常规链路
    ID: 0x0306	Byte:  8	自定义控制器与选手端交互数据	        发送方触发发送，频率上限为 30Hz	    自定义控制器→选手端							        -
    ID: 0x0307	Byte:103	选手端小地图接收路径数据		        频率上限为 1Hz					    哨兵/半自动控制机器人→对应操作手选手端		            常规链路
    ID: 0x0308	Byte: 34	选手端小地图接收机器人数据	            频率上限为 3Hz					    己方机器人→己方选手端						            常规链路
    ID: 0x0309	Byte: 30	自定义控制器接收机器人数据	            频率上限为 10Hz					    己方机器人→对应操作手选手端连接的自定义控制器	        图传链路
    ID: 0x0310  Byte: 150   机器人发送给自定义客户端的数据          频率上限维50Hz                      己方机器人→图传链路→对应操作手选手端连接的自定义客户端图传链路    

    ID：0x0F01  发送 1      设置图传出图信道                        频率上限为1Hz                       发送：机器人→图传发送端                                图传链路
                接收 1                                                                                  接收：图传发送端→机器人          
    ID：0x0F02  发送 0      查询当前出图信道                        频率上限为2Hz                       发送：机器人→图传发送端                                图传链路
                接收 1                                                                                  接收：图传发送端→机器人

    ID：0x0A01  Byte: 24    对方机器人的位置坐标                    频率上限为 10Hz                     信号发射源→雷达                                        雷达无线链路
    ID：0x0A02  Byte: 12    对方机器人的血量信息                    频率上限为 10Hz                     信号发射源→雷达                                        雷达无线链路
    ID: 0x0A03  Byte: 10    对方机器人的剩余发弹量信息              频率上限为 10Hz                     信号发射源→雷达                                        雷达无线链路
    ID: 0x0A04  Byte: 8     对方队伍的宏观状态信息                  频率上限为 10Hz                     信号发射源→雷达                                        雷达无线链路
    ID: 0x0A05  Byte: 36    对方各机器人当前增益效果                频率上限为 10Hz                     信号发射源→雷达                                        雷达无线链路
    ID: 0x0A06  Byte: 6     对方干扰波密钥                          频率上限为 10Hz                     信号发射源→雷达                                        雷达无线链路
    **********************************************************************************************************************************************/

//命令码ID,用来判断接收的是什么数据
typedef enum
{ 
	ID_game_state       			= 0x0001,//比赛状态数据
	ID_game_result 	   				= 0x0002,//比赛结果数据
	ID_game_robot_HP       			= 0x0003,//机器人血量数据
	
	ID_event_data  					= 0x0101,//场地事件数据 
	ID_referee_warning   	        = 0x0104,//裁判警告数据
	ID_dart_info 	                = 0x0105,//飞镖发射相关数据
	
	ID_robot_status     			= 0x0201,//机器人状态数据
	ID_power_heat_data    			= 0x0202,//实时功率热量数据
	ID_grobot_pos        		    = 0x0203,//机器人位置数据
	ID_buff     					= 0x0204,//机器人增益数据
	ID_hurt_data					= 0x0206,//伤害状态数据
	ID_shoot_data					= 0x0207,//实时射击数据
	ID_projectile_allowance			= 0x0208,//允许发弹量
	ID_rfid_status_t			    = 0x0209,//RFID模块状态
    ID_dart_client_cmd_t            = 0x020A,//飞镖选手端指令
    ID_ground_robot_position        = 0x020B,//地面机器人数据发给哨兵
    ID_radar_mark_data              = 0x020C,//雷达标记进度
	ID_sentry_info			        = 0x020D,//哨兵自主决策
    ID_radar_info                   = 0x020E,//雷达自主决策        
	
    ID_robot_interaction_data		= 0X0301,//机器人交互数据
    
    ID_map_command                  = 0x0303,//选手端下发数据
    ID_map_robot_data               = 0x0305,//选手端接收数据
    ID_map_data                     = 0x0307,//
    ID_custom_info                  = 0x0308,//
} CmdID;

/* ID: 0x0001  Byte:  3    比赛状态数据 
字节偏移量 大小 说明 
0 1
bit 0-3：比赛类型
• 1：RoboMaster 机甲大师超级对抗赛
• 2：RoboMaster 机甲大师高校单项赛
• 3：ICRA RoboMaster 高校人工智能挑战赛
• 4：RoboMaster 机甲大师高校联盟赛 3V3 对抗
• 5：RoboMaster 机甲大师高校联盟赛步兵对抗
bit 4-7：当前比赛阶段
• 0：未开始比赛
• 1：准备阶段
• 2：十五秒裁判系统自检阶段
• 3：五秒倒计时
• 4：比赛中
• 5：比赛结算中
1 2 当前阶段剩余时间，单位：秒
3 8 UNIX 时间，当机器人正确连接到裁判系统的 NTP 服务器后生效
*/
typedef __packed struct
{
	uint8_t game_type : 4;//0-3 bit：比赛类型
	uint8_t game_progress : 4;//4-7 bit：当前比赛阶段
	uint16_t stage_remain_time;//当前阶段剩余时间，单位 s
	uint64_t SyncTimeStamp;//机器人接收到该指令的精确 Unix 时间，当机载端收到有效的 NTP 服务器授时后生效
} ext_game_status_t;

/* ID: 0x0002  Byte:  1    比赛结果数据 */
typedef __packed struct 
{ 
	uint8_t winner;
} ext_game_result_t; 

/* ID: 0x0003  Byte:  2    机器人血量数据 */
typedef __packed struct
{
    uint16_t hero_HP;
    uint16_t engeer_HP;
    uint16_t balance_1_HP;
    uint16_t balance_2_HP;
    int16_t  damage_difference;     // 偏移8：己方全队总伤害 - 对方全队总伤害（有符号）
    uint16_t sentry_HP;
    uint16_t ally_outpost_HP;       // 偏移12：己方前哨站血量
    uint16_t ally_base_HP;          // 偏移14：己方基地血量
    uint16_t enemy_outpost_HP;      // 偏移16：对方前哨站血量
    uint16_t enemy_base_HP;         // 偏移18：对方基地血量
} ext_game_robot_HP_t; 

/* ID: 0x0101  Byte:  4    场地事件数据 */
typedef __packed struct 
{   //补给区
	uint32_t supply_no_include_Redemption :1;
    uint32_t supply_include_Redemption :1;
    uint32_t supply_RMUL :1;
    //能量机关
    uint32_t Small_Energy_Device :2;
    uint32_t Big_Energy_Device :2;
    //中央高地、梯形高地
    uint32_t Central_High_Ground :2;
    uint32_t Trapezoidal_Hill :2;
    //对方飞镖最后一次击中的时间
    uint32_t enemyDart_time :9;
    //敌方飞镖击中我方的目标
    uint32_t enemyDart_target :3;
    //中心增益点
    uint32_t Central_Buff_Point :2;
    //堡垒增益点
    uint32_t Defensive_Turret :2;
    //己方前哨站增益点
    uint32_t Outpost_BUFF :2;
    //己方基地增益点
    uint32_t Base_BUFF :1;
    
    uint32_t reserved :2;
    
} ext_event_data_t;  

/* ID: 0X0104  Byte: 2 		裁判警告信息：cmd_id (0x0104)。发送频率：警告发生后发送 */
typedef __packed struct
{
    uint8_t level;//最后一次受到判罚的等级 
    uint8_t offending_robot_id;//己方最后一次受到判罚的违规机器人 ID。（如红 1 机器人 ID 为 1，蓝1 机器人 ID 为 101）
    uint8_t count; //己方最后一次受到判罚的违规机器人对应判罚等级的违规次数。
} ext_referee_warning_t;

/*ID: 0X0105  Byte: 1		飞镖发射口倒计时：cmd_id (0x0105)*/
typedef __packed struct
{
    uint8_t dart_remaining_time; 
    uint16_t dart_info; 
} ext_dart_info_t;

/* ID: 0X0201  Byte: 15    机器人状态数据 */
typedef __packed struct
{
	uint8_t robot_id;
    uint8_t robot_level;
    uint16_t current_HP; 
    uint16_t maximum_HP;
    uint16_t shooter_barrel_cooling_value;
    uint16_t shooter_barrel_heat_limit;
    uint16_t chassis_power_limit; 
    float bullet_speed_limit;
    uint8_t power_management_gimbal_output : 1;
    uint8_t power_management_chassis_output : 1; 
    uint8_t power_management_shooter_output : 1;
} ext_robot_status_t;

/* ID: 0X0202  Byte: 14    实时功率热量数据 */
typedef __packed struct
{
 uint16_t reserved_1; 
 uint16_t reserved_2; 
 float reserved_3; 
 uint16_t buffer_energy; 
 uint16_t shooter_17mm_barrel_heat; 
 uint16_t shooter_42mm_barrel_heat; 
} ext_power_heat_data_t;


/* ID: 0x0203  Byte: 16    机器人位置数据 */
typedef __packed struct 
{   
    float x; 
    float y; 
    float vmm_angle; 
} ext_robot_pos_t; 

/* ID: 0x0204  Byte:  1    机器人增益数据 */
typedef __packed struct
{
    unsigned bit0  : 1; 
    unsigned bit1  : 1; 
    unsigned bit2  : 1; 
    unsigned bit3  : 1; 
    unsigned bit4  : 1; 
    unsigned bit5  : 1; 
    unsigned bit6  : 1; 
} buff;
typedef __packed union 
{
    buff   buffdata;
	uint8_t remain_energy;
} remain_energy_u;
typedef __packed struct
{
    uint8_t recovery_buff;                //bit 0 机器人血量补血状态 
    uint16_t cooling_buff;                 //bit 1：枪口热量冷却加速 
    uint8_t defence_buff;                 //bit 2：机器人防御加成 
    uint8_t vulnerability_buff;           //bit 3：机器人负防御增益
    uint16_t attack_buff;								 //bit 4;机器人攻击加成
    remain_energy_u remaining_energy;
}ext_buff_t;

/* ID: 0x0206  Byte:  1    伤害状态数据 */
typedef __packed struct 
{
    uint8_t armor_id : 4; 
    uint8_t HP_deduction_reason : 4;
	/*
	  0x0 装甲伤害扣血；
	  0x1 模块掉线扣血；
    0x2 超射速扣血；
	  0x3 超枪口热量扣血；
	  0x4 超底盘功率扣血；
	  0x5 装甲撞击扣血
	*/
} ext_hurt_data_t;

/* ID: 0x0207  Byte:  7    实时射击数据 */
typedef __packed struct
{
//	uint8_t bullet_type_17mm :1;
//    uint8_t bullet_type_42mm :1;
//	uint8_t shooter_number_17mm:1;
//    uint8_t reserved :1;
//    uint8_t shooter_number_42mm:1;
//	uint8_t launching_frequency; 
//    float initial_speed;
    uint8_t bullet_type; 
 uint8_t shooter_number; 
 uint8_t launching_frequency; 
 float initial_speed;
} ext_shoot_data_t;

/* ID: 0x0208	发送频率：10Hz 子弹剩余发射数 周期发送，所有机器人发送*/
typedef __packed struct
{
	uint16_t projectile_allowance_17mm;
	uint16_t projectile_allowance_42mm; 
    uint16_t remaining_gold_coin; 
    uint16_t projectile_allowance_fortress;
} ext_projectile_allowance_t;

/* ID: 0x0209    机器人 RFID 状态*/
/* 主结构体：完整对应5字节协议数据 */
//typedef __packed struct 
//{
//    /* 共用体：用于以不同方式访问前4个字节 */
//    __packed union
//    {
//        uint32_t status_value;                // 方式1：直接访问整个32位值
//         __packed struct 
//        {                              // 方式2：通过位域访问每一位
//            unsigned your_base                : 1; // bit0：己方基地增益点
//            unsigned your_central_high        : 1; // bit1：己方中央高地增益点
//            unsigned opp_central_high         : 1; // bit2：对方中央高地增益点
//            unsigned your_ladder_high         : 1; // bit3：己方梯形高地增益点
//            unsigned opp_ladder_high          : 1; // bit4：对方梯形高地增益点
//            unsigned your_slope_front         : 1; // bit5：己方地形跨越增益点（飞坡）（靠近己方一侧飞坡前）
//            unsigned your_slope_rear          : 1; // bit6：己方地形跨越增益点（飞坡）（靠近己方一侧飞坡后）
//            unsigned opp_slope_front          : 1; // bit7：对方地形跨越增益点（飞坡）（靠近对方一侧飞坡前）
//            unsigned opp_slope_rear           : 1; // bit8：对方地形跨越增益点（飞坡）（靠近对方一侧飞坡后）
//            unsigned your_under_central_high  : 1; // bit9：己方地形跨越增益点（中央高地下方）
//            unsigned your_above_central_high  : 1; // bit10：己方地形跨越增益点（中央高地上方）
//            unsigned opp_under_central_high   : 1; // bit11：对方地形跨越增益点（中央高地下方）
//            unsigned opp_above_central_high   : 1; // bit12：对方地形跨越增益点（中央高地上方）
//            unsigned your_under_highway       : 1; // bit13：己方地形跨越增益点（公路下方）
//            unsigned your_above_highway       : 1; // bit14：己方地形跨越增益点（公路上方）
//            unsigned opp_under_highway        : 1; // bit15：对方地形跨越增益点（公路下方）
//            unsigned opp_above_highway        : 1; // bit16：对方地形跨越增益点（公路上方）
//            unsigned your_fortress            : 1; // bit17：己方堡垒增益点
//            unsigned your_outpost             : 1; // bit18：己方前哨站增益点
//            unsigned your_supply_disjoint     : 1; // bit19：己方与兑换区不重叠的补给区/RMUL 补给区
//            unsigned your_supply_overlap      : 1; // bit20：己方与兑换区重叠的补给区
//            unsigned your_resource_island     : 1; // bit21：己方资源岛增益点
//            unsigned opp_resource_island      : 1; // bit22：对方资源岛增益点
//            unsigned central_rmul             : 1; // bit23：中心增益点（仅 RMUL 适用）
//            unsigned reserved_24_31           : 8; // bit24-bit31：保留位
//        } bits;
//    } status_union;

//    /* 第5个字节：同样使用共用体访问 */
//    __packed union
//     {
//        uint8_t status_2_value;                 // 方式1：直接访问整个字节
//         __packed struct 
//        {                                // 方式2：通过位域访问
//            unsigned opp_tunnel_low_ladder      : 1; // bit0：对方地形跨越增益点（隧道）（靠近对方梯形高地较低处）
//            unsigned opp_tunnel_high_ladder     : 1; // bit1：对方地形跨越增益点（隧道）（靠近对方梯形高地较高处）
//            unsigned reserved_2_7               : 6; // bit2-bit7：保留位
//        } bits_2;
//    } status_2_union;
//}ext_rfid_status_t;
typedef __packed struct
{
//	uint32_t rfid_status;	//bit 位值为 1/0 的含义：是否已检测到该增益点 RFID 卡
	uint32_t		Own_base_pt				: 1;	//bit 0：己方基地增益点	Own_base_point
	uint32_t		Own_cir_hland_pt		: 1;	//bit 1：己方中央高地增益点	Own_circular_highland_point
	uint32_t		Adv_cir_hland_pt		: 1;	//bit 2：对方中央高地增益点	Adverse_circular_highland_point
	uint32_t		Own_trapz_hland_pt		: 1;	//bit 3：己方梯形高地增益点	Own_trapezoidal_highland_point
	uint32_t		Adv_trapz_hland_pt		: 1;	//bit 4：对方梯形高地增益点	Adverse_trapezoidal_highland_point
	uint32_t		Own_pre_ramp_jump_pt	: 1;	//bit 5：己方地形跨越增益点（飞坡）（靠近己方一侧飞坡前）Own_pre_ramp_jump_point
	uint32_t		Own_post_ramp_jump_pt	: 1;	//bit 6：己方地形跨越增益点（飞坡）（靠近己方一侧飞坡后）Own_post_ramp_jump_point
	uint32_t		Adv_pre_ramp_jump_pt	: 1;	//bit 7：对方地形跨越增益点（飞坡）（靠近对方一侧飞坡前）Adverse_pre_ramp_jump_point
	uint32_t		Adv_post_ramp_jump_pt	: 1;	//bit 8：对方地形跨越增益点（飞坡）（靠近对方一侧飞坡后）Adverse_post_ramp_jump_point
	uint32_t		Own_under_ctr_hland_pt	: 1;	//bit 9：己方地形跨越增益点（中央高地下方）Own_under_central_highland_point
	uint32_t		Own_above_ctr_hland_pt	: 1;	//bit 10：己方地形跨越增益点（中央高地上方）Own_above_central_highland_point
	uint32_t		Adv_under_ctr_hland_pt	: 1;	//bit 11：对方地形跨越增益点（中央高地下方）Adverse_under_central_highland_point
	uint32_t		Adv_above_ctr_hland_pt	: 1;	//bit 12：对方地形跨越增益点（中央高地上方）Adverse_above_central_highland_point
	uint32_t		Own_under_highway_pt	: 1;	//bit 13：己方地形跨越增益点（公路下方）Own_under_Highway_point
	uint32_t		Own_above_highway_pt	: 1;	//bit 14：己方地形跨越增益点（公路上方）Own_above_Highway_point
	uint32_t		Adv_under_highway_pt	: 1;	//bit 15：对方地形跨越增益点（公路下方）Adverse_under_Highway_point
	uint32_t		Adv_above_highway_pt	: 1;	//bit 16：对方地形跨越增益点（公路上方）Adverse_above_Highway_point
	uint32_t		Own_fortress_pt			: 1;	//bit 17：己方堡垒增益点	Own_Fortress_Point
	uint32_t		Own_outpost_pt			: 1;	//bit 18：己方前哨站增益点	Own_outpost_Point
	uint32_t		Own_sup_non_ovlp_ex_rmul: 1;	//bit 19：己方与资源区不重叠的补给区/RMUL 补给区	Own_Supply_Non_Overlap_Exchange_or_RMUL_Supply
	uint32_t		Own_sup_ovlp_ex			: 1;	//bit 20：己方与资源区重叠的补给区	Own_Supply_Overlap_Exchange
	uint32_t		Own_assemble_pt	: 1;	//bit 21：己方装配增益点	Own_assemble_Point
	uint32_t		Adv_assemble_pt	: 1;	//bit 22：对方装配增益点	Adverse_assemble_Point
	uint32_t		Central_boost_point_RMUL: 1;	//bit 23：中心增益点（仅 RMUL 适用）	Central_Boost_Point_RMUL
	uint32_t		Adv_fortress_pt			: 1;	//bit 24：对方堡垒增益点	Adverse_Fortress_Point
	uint32_t          Adv_outpost_pt                : 1;  //bit 25：对方前哨站增益点
	uint32_t          Own_under_road_tunnel_pt   : 1;     //bit 26：己方地形跨越增益点（隧道）（靠近己方一侧公路区下方）
	uint32_t          Own_middle_road_tunnel_pt  : 1;     //bit 27：己方地形跨越增益点（隧道）（靠近己方一侧公路区中间）
	uint32_t          Own_above_road_tunnel_pt   : 1;     //bit 28：己方地形跨越增益点（隧道）（靠近己方一侧公路区上方）
	uint32_t          Own_under_highway_tunnel_pt   : 1;  //bit 29：己方地形跨越增益点（隧道）（靠近己方梯形高地较低处）
	uint32_t          Own_middle_highway_tunnel_pt  : 1;  //bit 30：己方地形跨越增益点（隧道）（靠近己方梯形高地较中间）
	uint32_t          Own_above_highway_tunnel_pt   : 1;  //bit 31：己方地形跨越增益点（隧道）（靠近己方梯形高地较高处）
	//注：所有 RFID 卡仅在赛内生效。在赛外，即使检测到对应的 RFID 卡，对应值也为 0。
	uint8_t          Adv_under_road_tunnel_pt   : 1;     //bit 0：对方地形跨越增益点（隧道）（靠近己方一侧公路区下方）
	uint8_t          Adv_middle_road_tunnel_pt  : 1;     //bit 1：对方地形跨越增益点（隧道）（靠近己方一侧公路区中间）
	uint8_t          Adv_above_road_tunnel_pt   : 1;     //bit 2：对方地形跨越增益点（隧道）（靠近己方一侧公路区上方）
	uint8_t          Adv_under_highway_tunnel_pt   : 1;  //bit 3：对方地形跨越增益点（隧道）（靠近己方梯形高地较低处）
	uint8_t          Adv_middle_highway_tunnel_pt  : 1;  //bit 4：对方地形跨越增益点（隧道）（靠近己方梯形高地较中间）
	uint8_t          Adv_above_highway_tunnel_pt   : 1;  //bit 5：对方地形跨越增益点（隧道）（靠近己方梯形高地较高处）
} ext_rfid_status_t;

/* ID: 0x020A	飞镖机器人客户端指令数据*/
typedef __packed struct
{
    uint8_t dart_launch_opening_status; 
    uint8_t reserved; 
    uint16_t target_change_time; 
    uint16_t latest_launch_cmd_time;
} ext_dart_client_cmd_t;

/* 
	
	交互数据，包括一个统一的数据段头结构，
	包含了内容 ID，发送者以及接受者的 ID 和内容数据段，
	整个交互数据的包总共长最大为 128 个字节，
	减去 frame_header,cmd_id,frame_tail 以及数据段头结构的 6 个字节，
	故而发送的内容数据段最大为 113。
	整个交互数据 0x0301 的包上行频率为 10Hz。

	机器人 ID：
	1，英雄(红)；
	2，工程(红)；
	3/4/5，步兵(红)；
	6，空中(红)；
	7，哨兵(红)；
	11，英雄(蓝)；
	12，工程(蓝)；
	13/14/15，步兵(蓝)；
	16，空中(蓝)；
	17，哨兵(蓝)。 
	客户端 ID： 
	0x0101 为英雄操作手客户端( 红) ；
	0x0102 ，工程操作手客户端 ((红 )；
	0x0103/0x0104/0x0105，步兵操作手客户端(红)；
	0x0106，空中操作手客户端((红)； 
	0x0111，英雄操作手客户端(蓝)；
	0x0112，工程操作手客户端(蓝)；
	0x0113/0x0114/0x0115，操作手客户端步兵(蓝)；
	0x0116，空中操作手客户端(蓝)。 
*/

/*ID: 0x020B己方机器人位置信息，场地围挡在红方补给站附近的交点为坐标原点，沿场地长边向蓝方为 X 轴正方向，沿场地短边
向红方停机坪为 Y 轴正方向，此数据信息只能被哨兵捕获。*/
typedef __packed struct
{
    float hero_x;														//己方英雄机器人位置 x 轴坐标，单位：m
    float hero_y;                            //己方英雄机器人位置 y 轴坐标
    float engineer_x;                        //己方工程机器人位置 x 轴坐标
    float engineer_y;                        //己方工程机器人位置 y 轴坐标
    float standard_3_x;                      //己方 3 号步兵机器人位置 x 轴坐标
    float standard_3_y;                      //己方 3 号步兵机器人位置 y 轴坐标
    float standard_4_x;                      //己方 4 号步兵机器人位置 x 轴坐标
    float standard_4_y;                      //己方 4 号步兵机器人位置 y 轴坐标
    float reserved_1; 
    float reserved_2;
}ext_ground_robot_position_t;

/* ID: 0x020C	雷达标记进度数据*/
typedef __packed struct 
{ 
    uint16_t enemy_hreo_vulnerable :1;
    uint16_t enemy_engineer_vulnerable :1;
    uint16_t enemy_balance_3_vulnerable :1;
    uint16_t enemy_balance_4_vulnerable :1;
    uint16_t enemy_senytry_vulnerable :1;
    
    uint16_t ally_hreo_mark_progress:1;
    uint16_t ally_engineer_mark_progress :1;
    uint16_t ally_balance_3_mark_progress :1;
    uint16_t ally_balance_4_mark_progress :1;
    uint16_t ally_senytry_mark_progress :1;
    
    uint16_t reserved_t :6;
}ext_radar_mark_data_t;

/* ID: 0x020D	哨兵自主指令数据*/
typedef __packed struct
{
    uint32_t sentry_allowed_ammunition_count : 11;  // bit0-10  除远程兑换外，已成功兑换的允许发弹量
    uint32_t sentry_redemptions_count         : 4;   // bit11-14 远程兑换允许发弹量的次数
    uint32_t sentry_blood_count              : 4;   // bit15-18 远程兑换血量的次数
    uint32_t sentry_Free_Revive              : 1;   // bit19    是否可以确认免费复活
    uint32_t sentry_Instant_Revive           : 1;   // bit20    是否可以兑换立即复活
    uint32_t sentry_Gold_Coins               : 10;  // bit21-30 立即复活需要花费的金币数
    uint32_t reserved                        : 1;   // bit31    保留

    uint16_t sentry_Out_of_Combat                  : 1;   // bit0     是否处于脱战状态
    uint16_t team_17mm_Ammunition_Left_for_Exchange: 11;  // bit1-11  队伍17mm允许发弹量的剩余可兑换数
    uint16_t sentry_posture                        : 2;   // bit12-13 当前姿态（1进攻 2防御 3移动）
    uint16_t Energy_Device                         : 1;   // bit14    己方能量机关是否可激活
    uint16_t sentry_enhanced_posture               : 1;   // bit15    当前姿态是否为强化姿态

    uint64_t attack_weaken_remaining   : 8;   // bit0-7   进攻姿态弱化前剩余时长（秒）
    uint64_t defense_weaken_remaining  : 8;   // bit8-15  防御姿态弱化前剩余时长
    uint64_t move_weaken_remaining     : 8;   // bit16-23 移动姿态弱化前剩余时长
    uint64_t reserved1                 : 8;   // bit24-31 保留
    uint64_t attack_enhanced_remaining : 8;   // bit32-39 强化进攻姿态剩余时长
    uint64_t defense_enhanced_remaining: 8;   // bit40-47 强化防御姿态剩余时长
    uint64_t move_enhanced_remaining   : 8;   // bit48-55 强化移动姿态剩余时长
    uint64_t reserved2                 : 8;   // bit56-63 保留
    
//    uint32_t sentry_allowed_ammunition_count :11;
//    uint32_t sentry_redemptions_count :4;
//    uint32_t sentry_blood_count :4;
//    uint32_t sentry_Free_Revive :1;
//    uint32_t sentry_Instant_Revive :1;
//    uint32_t sentry_Gold_Coins :10;
//    uint32_t reserved :1;

//    uint16_t no :1;
//    uint16_t mm17 :11;
//    uint16_t sentry_posture :2;
//    uint16_t Energy_Device :1;
//    uint16_t reserved_t :1;
} ext_sentry_info_t;

/* ID: 0x020E	雷达自主指令数据*/
typedef __packed struct
{
    uint8_t radar_info;
//    uint8_t double_vulnerable_nunber :2;
//    uint8_t enemy_if_double_vulnerable :1;
//    //后面未写
} ext_radar_info_t;

//交互数据接收信息：0x0301

typedef __packed struct
{
    uint16_t data_cmd_id;
    uint16_t send_ID;
    uint16_t receiver_ID;
    
    
}ext_student_interactive_header_data_t;

/* ID: 0X0303  */
typedef __packed struct 
{ 
    float target_position_x; 
    float target_position_y; 
    uint8_t cmd_keyboard; 
    uint8_t target_robot_id; 
    uint16_t cmd_source; 
}ext_map_command_t;
//0x0305
typedef __packed struct
{
    uint16_t opponent_hero_position_x;
    uint16_t opponent_hero_position_y;
    uint16_t opponent_engineer_position_x;
    uint16_t opponent_engineer_position_y;
    uint16_t opponent_infantry_3_position_x;
    uint16_t opponent_infantry_3_position_y;
    uint16_t opponent_infantry_4_position_x;
    uint16_t opponent_infantry_4_position_y;
    uint16_t opponent_aerial_position_x;
    uint16_t opponent_aerial_position_y;
    uint16_t opponent_sentry_position_x;
    uint16_t opponent_sentry_position_y;
    uint16_t ally_hero_position_x;
    uint16_t ally_hero_position_y;
    uint16_t ally_engineer_position_x;
    uint16_t ally_engineer_position_y;
    uint16_t ally_infantry_3_position_x;
    uint16_t ally_infantry_3_position_y;
    uint16_t ally_infantry4_position_x;
    uint16_t ally_infantry4_position_y;
    uint16_t ally_aerial_position_x;
    uint16_t ally_aerial_position_y;
    uint16_t ally_sentry_position_x;
    uint16_t ally_sentry_position_y;
}map_robot_data_t;

typedef struct 
{
	uint8_t	 Flag;			//帧头位数
	uint16_t data_len;		//数据长度
	uint16_t data_cnt;		//自加位置
	uint8_t	 data;
}Judge_FLAG;
/* ID: 0X0307  */
typedef __packed struct 
{
    uint8_t intention; 
    uint16_t start_position_x; 
    uint16_t start_position_y; 
    int8_t delta_x[49]; 
    int8_t delta_y[49]; 
    uint16_t sender_id; 
}ext_map_data_t;

typedef __packed struct
{
	ext_student_interactive_header_data_t interactive_data;
  float data1;
  float data2;
  float data3;
	uint8_t    mask;
} client_show_data_t;

typedef __packed struct
{
	xFrameHeader		FrameHeader;
	CmdID			    CmdID;

  __packed union
	{ 
		ext_game_status_t 			game_information;
		ext_hurt_data_t  			blood_changed_data;
		ext_shoot_data_t       		real_shoot_data;
		ext_power_heat_data_t   	real_powerheat_data;
		ext_event_data_t      		rfid_data;
		ext_game_result_t      		game_result_data;
		ext_buff_t         			get_buff_data;
		ext_robot_pos_t		        gameRobotPos;
		client_show_data_t  		client_show_data;
	}Data;
	uint16_t		CRC16;	//所有数据CRC校验
}Dateframe_t;//数据帧

/* 交互数据接收信息：0x0301  */

/* 
	客户端 客户端自定义数据：cmd_id:0x0301。内容 ID:0xD180
	发送频率：上限 10Hz


	1.	客户端 客户端自定义数据：cmd_id:0x0301。内容 ID:0xD180。发送频率：上限 10Hz 
	字节偏移量 	大小 	说明 				备注 
	0 			2 		数据的内容 ID 		0xD180 
	2 			2 		送者的 ID 			需要校验发送者机器人的 ID 正确性 
	4 			2 		客户端的 ID 		只能为发送者机器人对应的客户端 
	6 			4 		自定义浮点数据 1 	 
	10 			4 		自定义浮点数据 2 	 
	14 			4 		自定义浮点数据 3 	 
	18 			1 		自定义 8 位数据 4 	 

*/
typedef __packed struct 
{ 
	float data1; 
	float data2; 
	float data3; 
	uint8_t masks; 
} client_custom_data_t;


/* 
	学生机器人间通信 cmd_id 0x0301，内容 ID:0x0200~0x02FF
	交互数据 机器人间通信：0x0301。
	发送频率：上限 10Hz  

	字节偏移量 	大小 	说明 			备注 
	0 			2 		数据的内容 ID 	0x0200~0x02FF 
										可以在以上 ID 段选取，具体 ID 含义由参赛队自定义 
	
	2 			2 		发送者的 ID 	需要校验发送者的 ID 正确性， 
	
	4 			2 		接收者的 ID 	需要校验接收者的 ID 正确性，
										例如不能发送到敌对机器人的ID 
	
	6 			n 		数据段 			n 需要小于 113 

*/
typedef __packed struct 
{ 
	uint8_t data[10]; //数据段,n需要小于113
} robot_interactive_data_t;

/*-------------1. 交互数据接收信息： 0x0301。 发送频率：上限 10Hz---------
字节偏移量 大小 说明 备注
0 2 数据段的内容ID
2 2 发送者的ID   需要校验发送者的 ID 正确性，例如红 1 发送给红 5，此项需要校验红 1
4 2 接收者的ID  需要校验接收者的 ID 正确性，例如不能发送到敌对机器人的ID
6 x 内容数据段x 最大为 113
----*/

//帧头  命令码   数据段头结构  数据段   帧尾
//上传客户端
typedef __packed struct
{
	xFrameHeader   							txFrameHeader;//帧头
	uint16_t		 						CmdID;//命令码
	ext_student_interactive_header_data_t   dataFrameHeader;//数据段头结构
	client_custom_data_t  					clientData;//数据段
	uint16_t		 						FrameTail;//帧尾
}ext_SendClientData_t;
//客户端删除图形 机器人间通信：0x0301
typedef __packed struct
{
uint8_t operate_tpye; 
uint8_t layer; 
} ext_client_custom_graphic_delete_t;
//图形数据
typedef __packed struct
{ 
	uint8_t graphic_name[3]; 
	uint32_t operate_tpye:3; 
	uint32_t graphic_tpye:3; 
	uint32_t layer:4; 
	uint32_t color:4; 
	uint32_t start_angle:9;
	uint32_t end_angle:9;
	uint32_t width:10; 
	uint32_t start_x:11; 
	uint32_t start_y:11; 
	uint32_t radius:10; 
	uint32_t end_x:11; 
	uint32_t end_y:11; 
} graphic_data_struct_t;

//客户端绘制一个图形 机器人间通信：0x0301
typedef __packed struct
{
	graphic_data_struct_t grapic_data_struct;
} ext_client_custom_graphic_single_t;
//客户端绘制两个图形 机器人间通信：0x0301
typedef __packed struct
{
	graphic_data_struct_t grapic_data_struct[2];
} ext_client_custom_graphic_double_t;
//客户端绘制五个图形 机器人间通信：0x0301
typedef __packed struct
{
	graphic_data_struct_t grapic_data_struct[5];
} ext_client_custom_graphic_five_t;
//客户端绘制字符 机器人间通信
typedef __packed struct
{
	graphic_data_struct_t grapic_data_struct;
	uint8_t data[30];
} ext_client_custom_character_t;
//客户端绘制七个图形 机器人间通信
typedef __packed struct
{
	graphic_data_struct_t grapic_data_struct[7];
} ext_client_custom_graphic_seven_t;
//客户端下发信息
typedef __packed struct
{
	float target_position_x;
	float target_position_y;
	float target_position_z;
	uint8_t commd_keyboard;
	uint16_t target_robot_ID;
} ext_robot_command_t;

typedef __packed struct
{
	int16_t mouse_x;
	int16_t mouse_y;
	int16_t mouse_z;
	int8_t left_button_down;
	int8_t right_button_down;
	uint16_t keyboard_value;
	uint16_t reserved;
} ext_robot_commands_t;

typedef __packed struct
{
  uint8_t  data[64];
} user_to_server_t;

typedef __packed struct
{
  uint8_t  data[32];
} server_to_user_t;

/** 
  * @brief  the data structure receive from judgement
  */
/* data send (forward) */
/* data receive */


#define INFANTRY 1
#define HERO 2
#define ENGINEER 3
#define SENTINEL 4
typedef struct
{
	int16_t Voltage;
	int16_t Current;
	int16_t Supply_Num;
	int16_t ShooterHeat_17mm;
	int16_t ShooterHeat_42mm;
	int16_t Power;
	int16_t PowerBuffer;
	uint8_t level;
	uint8_t hurt_type;
}JudgementType;

typedef struct
{
	uint8_t 	graph_operate_type;
	uint8_t 	graph_type;
	uint8_t 	graph_name[5];
	uint8_t 	graph_color;
	uint8_t		graph_line_width;
	uint16_t graph_start_x;
	uint16_t graph_start_y;
	uint16_t graph_radius;
	uint16_t graph_dst_x;
	uint16_t graph_dst_y;
	uint8_t text_lengh;
	uint8_t text[30];
}Graph_Data_Type;

typedef struct
{
	int16_t 		ShootLevel;
	int16_t 		SuperCapacitorComment;
	float 					bullet_can_shoot;
	uint8_t 		State_Mask;
	uint8_t 		SuperCapacitorState;
	Graph_Data_Type Graph_Data ;
}SendToJudgementDataType;

//机器人交互信息
typedef __packed struct
{
	xFrameHeader   							txFrameHeader;//帧头
	uint16_t								CmdID;//命令码
	ext_student_interactive_header_data_t   dataFrameHeader;//数据段头结构
	robot_interactive_data_t  	 			interactData;//数据段
	uint16_t		 						FrameTail;//帧尾
}ext_CommunatianData_t;

void  judgement_data_handler(uint8_t *p_frame);

#endif
