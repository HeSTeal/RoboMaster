#include "judge_Task.h"
#include "usart.h"
#include "judgement_info.h"
// 正确的做法：FreeRTOS.h 必须第一个出现
#include "FreeRTOS.h"      // 必须第一个包含

// 其余头文件顺序任意
#include "task.h"          // 用于任务创建和管理
#include "queue.h"         // 用于队列操作
#include "protocol.h"
uint8_t    Judge_Buffer[JUDGE_BUFFER_LEN];//最终存储裁判系统信息的变量		
Judge_FLAG Judge_Flag;

extern QueueHandle_t RxCOM5;


void Judge_DataVerify(uint8_t *Buff)
{
    Dateframe_t	*frame;
    if(Buff!=NULL)
    { 
        frame=(Dateframe_t *)Buff;
	
        if(verify_crc16_check_sum((uint8_t *)frame, HEADER_LEN + CMD_LEN + frame->FrameHeader.DataLength + CRC_LEN))//进行整包校验
        {
            judgement_data_handler(Buff); 
        }
    }
}

uint8_t RES;//用来存储接收到的数据，应该与创建队列时每个项目的大小相等
void judge_task(void)
{
	xQueueReceive(RxCOM5,&RES,portMAX_DELAY);//RxCOM5是接收队列句柄，在usart.c中定义，也在其中被创建句柄，并从队列中读取数据到RES中	
	
	if(Judge_Flag.Flag==0 && RES==FRAME_HEADER)	//帧头的第一个字节
	{
		Judge_Flag.Flag=1;
		Judge_Buffer[0]=RES;
	}
	else if(Judge_Flag.Flag==1)				//长度,两字节，和下一个一起
	{
		Judge_Flag.Flag=2;
		Judge_Buffer[1]=RES;
	}
	else if(Judge_Flag.Flag==2)				//长度，和上一个一起
	{
		Judge_Flag.Flag=3;
		Judge_Buffer[2]=RES;
/****************通过Judge_Buffer【1】【2】来获取数据段长度***************/
		Judge_Flag.data_len=(uint16_t)(Judge_Buffer[2]<<8)|Judge_Buffer[1]+ LEN_CMDID + LEN_TAIL;//这里是后面接收的总长度，除帧头外
		if(Judge_Flag.data_len>100)
		{
			Judge_Flag.Flag=0;
		}
	}
	else if(Judge_Flag.Flag==3)				//包序号
	{
		Judge_Flag.Flag=4;
		Judge_Buffer[3]=RES;
	}
	else if(Judge_Flag.Flag==4)				//CRC8
	{
		Judge_Flag.Flag=5;
		Judge_Buffer[4]=RES;
		if (verify_crc8_check_sum( Judge_Buffer, HEADER_LEN ) != NULL)	//CRC8校验，通过比较接收数据的CRC8与裁判系统传来的是否一致，
		{
			Judge_Flag.data_cnt=0;
		}
		else				                //校验没有通过，重新开始
		{
			Judge_Flag.data_cnt=0;
			Judge_Flag.Flag=0;
		}
	}
	else if(Judge_Flag.Flag==5 && Judge_Flag.data_len>0)	//开始接受数据
	{
		Judge_Flag.data_len--;//他的长度不是只有数据段的，是id段+数据段+帧尾，跟上面一样，不是后面总长度的话，不能额直接进行CRC校验
		Judge_Buffer[LEN_HEADER+Judge_Flag.data_cnt++]=RES;
		if(Judge_Flag.data_len==0)
		{
			Judge_Flag.Flag=6;//ID段、数据段、帧尾都接收完后置6
		}	
	}
	else 
	{
		Judge_Flag.Flag=0;
		Judge_Flag.data_len=0;
		Judge_Flag.data_cnt=0;
	}
    
	if(Judge_Flag.Flag==6)
	{
		Judge_DataVerify(Judge_Buffer);		//帧头部分检测完毕，读取裁判系统数据
		Judge_Flag.Flag=0;					//其他数据清零
		Judge_Flag.data_len=0;
		Judge_Flag.data_cnt=0;
	}
	
 
}


