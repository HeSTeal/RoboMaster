#ifndef _JUDGE_TASK_H_
#define _JUDGE_TASK_H_

#include "main.h"

#define FRAME_HEADER   0xA5   
#define JUDGE_BUFFER_LEN 		100
    
void judge_task(void);
void Judge_DataVerify(uint8_t *Buff);

#endif


