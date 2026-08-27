#ifndef __RGB_H
#define __RGB_H

#include "main.h"

typedef enum
{
    RGB_OFF,
    RGB_RED,
    RGB_GREEN,
    RGB_BULE,
    RGB_YELLOW,//R GÁÁ BÃð  °µ»ÆÉ«
    RGB_CYAN,//RÃð  G  BÁÁ  °µÇàÉ«
    RGB_MAGENTA,//R  BÁÁ GÃð°µºìÉ«
    RGB_WHITE//R G BÁÁ      °×É«
}RGB_enum;


void RGB_action(RGB_enum RGB);
    
#endif

