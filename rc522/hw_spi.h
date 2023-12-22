/*
 * @author: WangQiWei
 * @Date: 2023-12-05 23:51:36
 * @LastEditors: WangQiWei
 * @LastEditTime: 2023-12-21 17:02:47
 */
#ifndef __HW_SPI_H__
#define __HW_SPI_H__

#include "gd32f4xx.h"

/*RC522的端口和引脚宏定义*/
#define CS_PORT             GPIOA
#define SCK_PORT            GPIOA
#define MISO_PORT           GPIOA
#define MOSI_PORT           GPIOB
#define RESET_PORT          GPIOE

#define CS_PIN              GPIO_PIN_4     
#define SCK_PIN             GPIO_PIN_5
#define MISO_PIN            GPIO_PIN_6
#define MOSI_PIN            GPIO_PIN_5
#define RESET_PIN           GPIO_PIN_10

/*CS和RESET的宏定义*/
#define CS_Enable()         gpio_bit_reset(CS_PORT,CS_PIN) //低电平选中
#define CS_Disable()        gpio_bit_set(CS_PORT,CS_PIN)
#define Reset_Enable()         gpio_bit_reset(RESET_PORT,RESET_PIN)
#define Reset_Disable()        gpio_bit_set(RESET_PORT,RESET_PIN)


void HardWare_SPI_Init(void);
uint8_t HardWare_SPI_SwapByte(uint8_t byte);
void HardWare_SPI_Start(void);
void HardWare_SPI_Stop(void);


#endif

