#include "RGB.h"

RGB_enum RGB_color;
//根据原理图，LED的阳极接VSS，阴极接IO口，所以使用开漏输出，初始化配置为高电平，IO表现为高阻抗，初始状态为灭
/*PB7 R 
  PB4 G
  PB3 B*/
//这个RGB行为函数，使用时传递一个RGB_enum类型的枚举变量即可，根据枚举变量的名称亮不同的颜色
void RGB_action(RGB_enum RGB)
{
    switch(RGB)
    {
        case RGB_OFF:
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
        break;                    
                                  
        case RGB_RED:             
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
        break;                    
                                  
        case RGB_GREEN:           
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
        break;                    
                                  
        case RGB_BULE:            
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
        break;                    
                                  
        case RGB_YELLOW:          
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
        break;                    
                                  
        case RGB_CYAN:            
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
        break;                    
                                  
        case RGB_MAGENTA:         
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
        break;                    
                                  
        case RGB_WHITE:           
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
        break;                    
                                  
        default:                  
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
        break;
            
    }
}


