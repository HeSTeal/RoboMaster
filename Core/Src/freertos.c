/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ins_task.h"
#include "judge_Task.h"
#include "can.h"
#include "Chassis_task.h"
#include "NAVI_Task.h"
#include "RGB.h"
#include "iwdg.h"
#include "tim.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern RGB_enum RGB_color;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myIMU */
osThreadId_t myIMUHandle;
const osThreadAttr_t myIMU_attributes = {
  .name = "myIMU",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityRealtime3,
};
/* Definitions for myChassis */
osThreadId_t myChassisHandle;
const osThreadAttr_t myChassis_attributes = {
  .name = "myChassis",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh1,
};
/* Definitions for myNAVI */
osThreadId_t myNAVIHandle;
const osThreadAttr_t myNAVI_attributes = {
  .name = "myNAVI",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh7,
};
/* Definitions for myJudgement */
osThreadId_t myJudgementHandle;
const osThreadAttr_t myJudgement_attributes = {
  .name = "myJudgement",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh5,
};
/* Definitions for myStrategic */
osThreadId_t myStrategicHandle;
const osThreadAttr_t myStrategic_attributes = {
  .name = "myStrategic",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityHigh3,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartIMU(void *argument);
void StartChassis(void *argument);
void StartNAVI(void *argument);
void StartJument(void *argument);
extern void StartStrategic(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myIMU */
  myIMUHandle = osThreadNew(StartIMU, NULL, &myIMU_attributes);

  /* creation of myChassis */
  myChassisHandle = osThreadNew(StartChassis, NULL, &myChassis_attributes);

  /* creation of myNAVI */
  myNAVIHandle = osThreadNew(StartNAVI, NULL, &myNAVI_attributes);

  /* creation of myJudgement */
  myJudgementHandle = osThreadNew(StartJument, NULL, &myJudgement_attributes);

  /* creation of myStrategic */
  myStrategicHandle = osThreadNew(StartStrategic, NULL, &myStrategic_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartIMU */
/**
* @brief Function implementing the myIMU thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartIMU */
void StartIMU(void *argument)
{
  /* USER CODE BEGIN StartIMU */
//  INS_Init();
    /* Infinite loop */
    for (;;)
    {
        CAN1_Send_up();
        CAN1_Send_up_t();
        CAN1_Send_up_tt();
//        INS_Task();
        osDelay(1);
    }
  /* USER CODE END StartIMU */
}

/* USER CODE BEGIN Header_StartChassis */
/**
* @brief Function implementing the myChassis thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartChassis */
void StartChassis(void *argument)
{
  /* USER CODE BEGIN StartChassis */
    Chassis_PID_Init();
    MX_IWDG_Init();
  /* Infinite loop */
  for(;;)
  {
      Chassis_movment();
      HAL_IWDG_Refresh(&hiwdg);
    osDelay(1);
  }
  /* USER CODE END StartChassis */
}

/* USER CODE BEGIN Header_StartNAVI */
/**
* @brief Function implementing the myNAVI thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartNAVI */
void StartNAVI(void *argument)
{
  /* USER CODE BEGIN StartNAVI */
  /* Infinite loop */
  for(;;)
  {
      RGB_action(RGB_color);
      NAVI_RX();
      NAVI_TX();
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 500);
    osDelay(1);
  }
  /* USER CODE END StartNAVI */
}

/* USER CODE BEGIN Header_StartJument */
/**
* @brief Function implementing the myJudgement thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartJument */
void StartJument(void *argument)
{
  /* USER CODE BEGIN StartJument */
  /* Infinite loop */
  for(;;)
  {
      judge_task();
      
//    osDelay(1);
  }
  /* USER CODE END StartJument */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

