#ifndef __RS485_H_
#define	__RS485_H_

#include "main.h"

#define RS485_DIR_GPIO_Port 	GPIOC
#define RS485_DIR_Pin 			GPIO_PIN_9

#define RS485_TX_EN()	HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_SET);
#define RS485_RX_EN()	HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_RESET);

typedef __packed struct
{
    __packed struct
    {
        int16_t ch[5];
        char s[2];
    } rc;
} RC_ctrl_t;

void RS485_IdleCallback(void);
void RS485_TO_RC(volatile const uint8_t *RS485_buf, RC_ctrl_t *rc_ctrl);
void RS485_Init(void);

#endif

