#ifndef __485PORT_H
#define __485PORT_H	 
//#include "tim.h"
//#include "delay.h"
//#include "wdg.h"
#include "gd32f4xx.h"

#define  RS485_DE_PORT  GPIOA
#define  RS485_DE_PIN   GPIO_PIN_8
#define  RS485_DE_RCU   RCU_GPIOA
#define  RS485_DE_DISABLE()  gpio_bit_reset(RS485_DE_PORT,RS485_DE_PIN)
#define  RS485_DE_ENABLE()   gpio_bit_set(RS485_DE_PORT,RS485_DE_PIN)

void    Rs485Port_Init(void);
void    RS485_SendData(uint8_t *buf,uint16_t len);
#endif
