#include "485Port.h"	   
#include "gd32f4xx.h"
#include "gd32f4xx_gpio.h"
#include "gd32f4xx_spi.h"
#include "gd32f4xx_spi.h"
#include "systick.h"
#include "gd32f4xx_usart.h"


/******************************************************************************
*函数说明：Rs485端口初始化
*函数名称：Rs485Port_Init(void)
*输入参数：无
*输出参数：无
*返回值：  无
******************************************************************************/
void   Rs485Port_Init(void)
{
	
	     //RS485 方向控制引脚
	     rcu_periph_clock_enable(RS485_DE_RCU);
	     gpio_mode_set(RS485_DE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RS485_DE_PIN);
       gpio_output_options_set(RS485_DE_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, RS485_DE_PIN);
	     RS485_DE_ENABLE();
	
	     
	     //串口初始化
	     rcu_periph_clock_enable(RCU_USART0);//使能USART0时钟

	     rcu_periph_clock_enable(RCU_GPIOA);
	     gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);    //复用功能7
       gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);//PA9配置成串口输出
       gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_9);
    
	     rcu_periph_clock_enable(RCU_GPIOA);
       gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);   //复用功能7
       gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);//PA10配置成串口输入
       gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_10);
	
	     usart_deinit(USART0);    // 串口复位
       usart_word_length_set(USART0, USART_WL_8BIT);  // 字长
       usart_stop_bit_set(USART0, USART_STB_1BIT);    // 停止位
       usart_parity_config(USART0, USART_PM_NONE);
       usart_baudrate_set(USART0, 9600U);     // 波特率
       //usart_receive_config(USART1, USART_RECEIVE_ENABLE);        // 接收使能
       usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);      // 发送使能
       usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
       usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
       usart_enable(USART0);           // 串口使能
       //nvic_irq_enable(USART0_IRQn,1,0);
}

/******************************************************************************
*函数说明：Rs485发送函数
*函数名称：RS485_SendData(uint8_t *buf,uint16_t len)
*输入参数：*buf：待发送数据缓存区    len：待发送数据字节数
*输出参数：无
*返回值：  无
******************************************************************************/
void RS485_SendData(uint8_t *buf,uint16_t len)
{
     uint8_t t;
     //SET_485_TX        // 发送使能
     for(t=0;t<len;t++)      
     {           
         while(usart_flag_get(USART0, USART_FLAG_TC) == RESET);  
         usart_data_transmit(USART0,buf[t]);
     }     
    // while(usart_flag_get(USART1, USART_FLAG_TC) == RESET);   
       
         
     //    usart_interrupt_enable(USART1, USART_INT_RBNE);  // 接收中断使能
         //SET_485_RX
}
