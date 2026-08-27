/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan1;

extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_CAN1_Init(void);
void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */
typedef struct
{
    uint16_t 	volt;
    uint16_t	power;
    uint16_t	current;
} Super_power_t;
typedef struct 
{
	uint16_t power;
	uint16_t flag;
	uint16_t buffer_power;
}Send_Data;

typedef union 
{
  Send_Data TX_data;
  uint8_t Array_Tx_data[sizeof(Send_Data)];
}Tx_Union_data;
void CAN1_Filter_Config(void);
void CAN2_Filter_Config(void);
void CAN2_Send(int32_t out1,int32_t out2,int32_t out3,int32_t out4);
void Get_RC_info(uint8_t *RxData);
void Get_YAW_info(uint8_t *RxData);
void CAN1_Send_up(void);
void CAN1_Send_up_t(void);
void CAN1_Send_up_tt(void);
float uint_to_float(int x_int, float x_min, float x_max, int bits);
void Get_Super_power(uint8_t *RxData);
void Can_Send_TO_Superpower(CAN_HandleTypeDef *hcan, int16_t power, int16_t i,int16_t buffer_power);
void Get_info(uint8_t *RxData);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

