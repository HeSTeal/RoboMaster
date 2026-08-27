/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "Motor.h"
#include "RC.h"
#include "RS485.h"
#include "Chassis_task.h"
#include "judgement_info.h"
#include "NAVI_Task.h"
#include "RGB.h"

extern motor_measure_t M3508[4];
extern RC_ctrl_t rc_ctrl;
extern rc_mode_t Sentry_Mode;
extern uint8_t flag_reset;
extern Yaw_relative_union Yaw_motor;
extern ext_robot_status_t RobotStatust;
extern ext_power_heat_data_t PowerHeatData;
extern ext_sentry_info_t SentryInfo;	
extern ext_game_status_t GameState;	
extern ext_shoot_data_t	ShootData;
extern ext_hurt_data_t HurtData;
extern float AGV_w;//������ת���ٶȣ���/s
extern NAVI_RX_union NAVI_Data;
extern RGB_enum RGB_color;
unsigned char *ptr=0;
unsigned char *ptr_t=0;
float error_HP=0,last_HP=0;
float xj=0;
damiao_struct Yaw_4310_motor;
motor_measure_t Push_motor;
Super_power_t Super_power;//����
uint8_t zimiao_mode=0;//1�����ˣ�2����
/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 3;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_9TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = ENABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = ENABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */
    CAN1_Filter_Config();
  /* USER CODE END CAN1_Init 2 */

}
/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 3;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_9TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan2.Init.TimeTriggeredMode = ENABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = ENABLE;
  hcan2.Init.AutoRetransmission = ENABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */
    CAN2_Filter_Config();
  /* USER CODE END CAN2_Init 2 */

}

static uint32_t HAL_RCC_CAN1_CLK_ENABLED=0;

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    HAL_RCC_CAN1_CLK_ENABLED++;
    if(HAL_RCC_CAN1_CLK_ENABLED==1){
      __HAL_RCC_CAN1_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB5     ------> CAN2_RX
    PB6     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_RX1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX1_IRQn);
  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
  else if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    HAL_RCC_CAN1_CLK_ENABLED--;
    if(HAL_RCC_CAN1_CLK_ENABLED==0){
      __HAL_RCC_CAN1_CLK_DISABLE();
    }

    /**CAN2 GPIO Configuration
    PB5     ------> CAN2_RX
    PB6     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5|GPIO_PIN_6);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void CAN1_Filter_Config(void)
{
    CAN_FilterTypeDef  sFilterConfig;
    
    sFilterConfig.FilterBank = 0;                     
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;  
	sFilterConfig.FilterIdLow = 0x0000;   
	sFilterConfig.FilterMaskIdHigh = 0x0000;  
	sFilterConfig.FilterMaskIdLow = 0x0000;  
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;          
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK)
    {
        while(1){}
    }

    if (HAL_CAN_Start(&hcan1) != HAL_OK)
    {
        while(1){}
    }

    if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        while(1){}
    }
}
void CAN2_Filter_Config(void)
{
    CAN_FilterTypeDef sFilterConfig;
    
    sFilterConfig.FilterBank = 14;                     
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000; 
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO1;
    sFilterConfig.FilterActivation = ENABLE;          
    sFilterConfig.SlaveStartFilterBank = 14;
    
    if (HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig) != HAL_OK)
    {
        while(1){}
    }

    if (HAL_CAN_Start(&hcan2) != HAL_OK)
    {
        while(1){}
    }

    if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK)
    {
        while(1){}
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{                             
    CAN_RxHeaderTypeDef RxHeader1;
    static uint8_t RxData[8];
    HAL_StatusTypeDef status;
    status = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader1, RxData);
    if (status != HAL_OK) 
    {
        Error_Handler();
        return;
    }
            
    switch(RxHeader1.StdId)
    {
        case 0x301: Get_RC_info(RxData); break;
        case 0x302: Get_info(RxData); break;
		case 0x11 : damiao_get_motor_measure(&Yaw_4310_motor,RxData);break;
        case 0x201: get_motor_measure(&Push_motor,RxData);break;
                
        default:
        break;
    }    
   
}
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef RxHeader2;
    static uint8_t RxData[8];
    HAL_StatusTypeDef status;
    status = HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO1, &RxHeader2, RxData);
    if (status != HAL_OK) 
    {
        Error_Handler();
        return;
    }
            
    switch(RxHeader2.StdId)
    {
        case 0x201: get_motor_measure(&M3508[0],RxData); break;
		case 0x202: get_motor_measure(&M3508[1],RxData); break;
		case 0x203: get_motor_measure(&M3508[2],RxData); break;
		case 0x204: get_motor_measure(&M3508[3],RxData); break;
        case 0x300: Get_Super_power(RxData);break;

        default:
			break;
	}           
    
}

void CAN2_Send(int32_t out1,int32_t out2,int32_t out3,int32_t out4)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];
    uint32_t TxMailbox;
    
    TxHeader.StdId = 0x200;         
    TxHeader.ExtId = 0x0000;        
    TxHeader.IDE = CAN_ID_STD;        
    TxHeader.RTR = CAN_RTR_DATA;     
    TxHeader.DLC = 8;                
    TxHeader.TransmitGlobalTime = DISABLE;

    TxData[0] = (uint8_t)(out1 >> 8);
    TxData[1] = (uint8_t)out1;
    TxData[2] = (uint8_t)(out2 >> 8);
    TxData[3] = (uint8_t)out2;
    TxData[4] = (uint8_t)(out3 >> 8);
    TxData[5] = (uint8_t)out3;
    TxData[6] = (uint8_t)(out4 >> 8);
    TxData[7] = (uint8_t)out4;

    if (HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
        //Error_Handler();
		HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox); 
    }
}

void Get_Super_power(uint8_t *RxData)
{
    Super_power.volt = (uint16_t)(RxData[1] << 8 | RxData[0]);
    Super_power.power = (uint16_t)(RxData[3] << 8 | RxData[2]);
    Super_power.current = (uint16_t)(RxData[5] << 8 | RxData[4]);
}

void Get_RC_info(uint8_t *RxData)
{
    Sentry_Mode=(rc_mode_t)RxData[0];//�ڱ�ģʽ
    rc_ctrl.rc.ch[0]=(uint16_t)(RxData[1] << 8 | RxData[2]);//ң����ͨ��
    rc_ctrl.rc.ch[1]=(uint16_t)(RxData[3] << 8 | RxData[4]);
    rc_ctrl.rc.ch[4]=(uint16_t)(RxData[5] << 8 | RxData[6]);
    if(RxData[7]==11)//�ڰ˸�λ
    {
        flag_reset=111;
    }
    else
        flag_reset=0;
}
void Get_info(uint8_t *RxData)
{
    zimiao_mode=RxData[0];
    
}
void Get_YAW_info(uint8_t *RxData)
{
    for(int i=0;i<8;i++)
    {
        Yaw_motor.Yaw_relativeArray[i]=RxData[i];
    }
}
uint8_t aa_t=0;
void CAN1_Send_up(void)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];
    uint32_t TxMailbox;
    
    TxHeader.StdId = 0x303;         
    TxHeader.ExtId = 0x0000;        
    TxHeader.IDE = CAN_ID_STD;        
    TxHeader.RTR = CAN_RTR_DATA;     
    TxHeader.DLC = 8;                
    TxHeader.TransmitGlobalTime = DISABLE;   

    if(RobotStatust.robot_id==7)
        TxData[0]=1;//����
    else if(RobotStatust.robot_id==107)
        TxData[0]=2;//�췽
    else
        TxData[0]=3;//����ID�����ڱ���
    
    TxData[1] = PowerHeatData.shooter_17mm_barrel_heat & 0xFF;//��ǰǹ������
    TxData[2] = (PowerHeatData.shooter_17mm_barrel_heat >> 8) & 0xFF;//��ǰǹ������

    TxData[3]=SentryInfo.sentry_posture+3*SentryInfo.sentry_enhanced_posture;
    TxData[4]=GameState.game_progress;
    
    error_HP=last_HP-RobotStatust.current_HP;
    last_HP=RobotStatust.current_HP;
    
    if(HurtData.HP_deduction_reason==0&&error_HP>3)
    {
        TxData[5]=HurtData.armor_id;//�ܻ�װ�װ�ID
        xj = DWT_GetTimeline_s();
    }
    else
        TxData[5]=5;
    if(DWT_GetTimeline_s() - xj<0.2f)
    {
        TxData[5]=HurtData.armor_id;//�ܻ�װ�װ�ID
    }
    else
        TxData[5]=5;

    if(NAVI_Data.NAVI_RX_Data.chassis_type==1)
        TxData[6]=1;//1�ǵ�ͷ
    else if(NAVI_Data.NAVI_RX_Data.chassis_type==2||NAVI_Data.NAVI_RX_Data.chassis_type==0)
        TxData[6]=2;//2��Ѳ��

    if(RGB_color==RGB_WHITE)
        TxData[7]=3;//发3有导航数据
    else
        TxData[7]=1;
    
    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
        //Error_Handler();
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox); 
    }
}
void CAN1_Send_up_t(void)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];
    uint32_t TxMailbox;
    
    TxHeader.StdId = 0x304;         
    TxHeader.ExtId = 0x0000;        
    TxHeader.IDE = CAN_ID_STD;        
    TxHeader.RTR = CAN_RTR_DATA;     
    TxHeader.DLC = 8;                
    TxHeader.TransmitGlobalTime = DISABLE;   

    ptr=(unsigned char *)(&AGV_w);
	TxData[0]=*ptr;
	TxData[1]=*(ptr+1);
	TxData[2]=*(ptr+2);
	TxData[3]=*(ptr+3);
    
    TxData[4] = RobotStatust.shooter_barrel_cooling_value & 0xFF;//���������ȴ���ʣ�30/�� 
    TxData[5] = (RobotStatust.shooter_barrel_cooling_value >> 8) & 0xFF;
    TxData[6] = RobotStatust.shooter_barrel_heat_limit & 0xFF;        // ���ֽ� ����������ޣ�260
    TxData[7] = (RobotStatust.shooter_barrel_heat_limit >> 8) & 0xFF; // ���ֽ�
    
    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
        //Error_Handler();
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox); 
    }
}

void CAN1_Send_up_tt(void)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];
    uint32_t TxMailbox;
    
    TxHeader.StdId = 0x305;         
    TxHeader.ExtId = 0x0000;        
    TxHeader.IDE = CAN_ID_STD;        
    TxHeader.RTR = CAN_RTR_DATA;     
    TxHeader.DLC = 8;                
    TxHeader.TransmitGlobalTime = DISABLE;   

	ptr_t=(unsigned char *)(&ShootData.initial_speed);
	TxData[0]=*ptr_t;
	TxData[1]=*(ptr_t+1);
	TxData[2]=*(ptr_t+2);
	TxData[3]=*(ptr_t+3);
    
    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
        //Error_Handler();
		HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox); 
    }
}
 /**
  * @brief  ���ø������ݵȱ���ת��������
  * @param  x_int     	Ҫת�����޷�������
  * @param  x_min      Ŀ�긡��������Сֵ
  * @param  x_max    	Ŀ�긡���������ֵ
  * @param  bits      	�޷���������λ��
  */
float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
    /// converts unsigned int to float, given range and number of bits ///
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int) * span / ((float)((1 << bits) - 1)) + offset;
}
Tx_Union_data Can_Tx_Data;
void Can_Send_TO_Superpower(CAN_HandleTypeDef *hcan, int16_t power, int16_t i,int16_t buffer_power)
{
	  CAN_TxHeaderTypeDef TxHeader;
  	  uint8_t Data[8];
      uint32_t TxMailbox;
      Can_Tx_Data.TX_data.power = power;
	  Can_Tx_Data.TX_data.flag  = i;
	  Can_Tx_Data.TX_data.buffer_power = buffer_power;
	  
	  TxHeader.StdId = 0x222;
      TxHeader.ExtId = 0;
      TxHeader.IDE = CAN_ID_STD;
      TxHeader.RTR = CAN_RTR_DATA;
      TxHeader.DLC = 8;
      TxHeader.TransmitGlobalTime = DISABLE;
	  for(int i=0;i<sizeof(Send_Data);i++)
      {
            Data[i] = Can_Tx_Data.Array_Tx_data[i];
	  }
		HAL_CAN_AddTxMessage(&hcan2,&TxHeader, Data, &TxMailbox);
}

 
/* USER CODE END 1 */

