#include "judgement_info.h"
#include "protocol.h"
#include "string.h"
/*****************系统数据定义**********************/
ext_game_status_t       			GameState;					//0x0001
ext_game_result_t            		GameResult;					//0x0002
ext_game_robot_HP_t          		GameRobotSurvivors;			//0x0003

ext_event_data_t        			EventData;					//0x0101
ext_referee_warning_t               RefereeWarning;             //0x0104
ext_dart_info_t                     DartInfo;                   //0x0105

ext_robot_status_t			  	    RobotStatust;				//0x0201
ext_power_heat_data_t		  		PowerHeatData;				//0x0202
ext_robot_pos_t		        		RobotPos;				    //0x0203
ext_buff_t							Buff;					    //0x0204
ext_hurt_data_t					    HurtData;					//0x0206
ext_shoot_data_t					ShootData;					//0x0207
ext_projectile_allowance_t			ProjectileAllowance;		//0x0208
ext_rfid_status_t           		RfidStatus;                 //0x0209
ext_dart_client_cmd_t               DartClientCmd;              //0x020A
ext_ground_robot_position_t   		GroundRobotPosition;		//0x020B
ext_radar_mark_data_t               RadarMarkData;              //0x020C
ext_sentry_info_t					SentryInfo;				    //0x020D
ext_radar_info_t                    RadarInfo;                  //0X020E

ext_map_command_t                   MapCommand;                 //0x0303
/**用户发送**/

ext_SendClientData_t      ShowData;			//客户端信息
ext_CommunatianData_t     CommuData;		//队友通信信息
/****************************************************/
uint8_t Judge_Self_ID;//当前机器人的ID
uint16_t Judge_SelfClient_ID;//发送者机器人对应的客户端ID
uint16_t cmd_id;

void judgement_data_handler(uint8_t *p_frame)
{
    frame_header_t *p_header = (frame_header_t*)p_frame;
    memcpy(p_header, p_frame, HEADER_LEN);//复制到哪里，从哪里复制，复制长度

    uint16_t data_length = p_header->data_length;									//数据长度
	cmd_id = *(uint16_t *)(p_frame + HEADER_LEN);						//命令         这里其实就是取的Judge_Buffer的第六个和第七个数据
    uint8_t *data_addr = p_frame + HEADER_LEN + CMD_LEN;			//数据地址
  
    switch (cmd_id)
    {
   	    case ID_game_state:        			//0x0001
			memcpy(&GameState, (data_addr), data_length);
		break;
		
		case ID_game_result:          		//0x0002
			memcpy(&GameResult, (data_addr), data_length);
		break;
		
		case ID_game_robot_HP:               //0x0003
			memcpy(&GameRobotSurvivors,(data_addr), data_length);
		break;
		
		case ID_event_data:    				//0x0101
			memcpy(&EventData, (data_addr), data_length);
		break;
        
        case ID_referee_warning:    		//0x0104
			memcpy(&RefereeWarning, (data_addr), data_length);
		break;
        
        case ID_dart_info:    		        //0x0105
			memcpy(&DartInfo, (data_addr), data_length);
		break;
        
		case ID_robot_status:      		    //0x0201
			memcpy(&RobotStatust,(data_addr), data_length);
		break;
		
		case ID_power_heat_data:      		//0x0202
			memcpy(&PowerHeatData,  (data_addr), data_length);
		break;
		
		case ID_grobot_pos:      		    //0x0203
			memcpy(&RobotPos,(data_addr), data_length);
		break;
		
		case ID_buff:      			        //0x0204
			memcpy(&Buff,(data_addr), data_length);
		break;
		
		case ID_hurt_data:      			//0x0206
			memcpy(&HurtData, (data_addr), data_length);
		break;

		case ID_shoot_data:      			//0x0207
			memcpy(&ShootData,(data_addr), data_length);
		break;
        
		case ID_projectile_allowance:      	//0x0208
			memcpy(&ProjectileAllowance,(data_addr), data_length);
		break;
        
        case ID_rfid_status_t:                  //0x0209
            memcpy(&RfidStatus,(data_addr), data_length);
		break;

        case ID_dart_client_cmd_t:                  //0x020A
            memcpy(&DartClientCmd,(data_addr), data_length);
		break;
         
        case ID_ground_robot_position:                  //0x020B
            memcpy(&GroundRobotPosition,(data_addr), data_length);
		break; 
        
        case ID_radar_mark_data:                  //0x020C
            memcpy(&RadarMarkData,(data_addr), data_length);
		break; 
        
        case ID_sentry_info:                  //0x020D
            memcpy(&SentryInfo,(data_addr), data_length);
        break;

        case ID_radar_info:                  //0x020E
            memcpy(&RadarInfo,(data_addr), data_length);
        break;
		case ID_map_command:                 //0x0303
            memcpy(&MapCommand,(data_addr), data_length);
        break;
        
		default : break;
  }
}



