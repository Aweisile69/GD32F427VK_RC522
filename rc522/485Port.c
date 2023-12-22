#include "485Port.h"	   
#include "gd32f4xx.h"
#include "gd32f4xx_gpio.h"
#include "gd32f4xx_spi.h"
#include "gd32f4xx_spi.h"
#include "systick.h"
#include "gd32f4xx_usart.h"


/******************************************************************************
*����˵����Rs485�˿ڳ�ʼ��
*�������ƣ�Rs485Port_Init(void)
*�����������
*�����������
*����ֵ��  ��
******************************************************************************/
void   Rs485Port_Init(void)
{
	
	     //RS485 �����������
	     rcu_periph_clock_enable(RS485_DE_RCU);
	     gpio_mode_set(RS485_DE_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, RS485_DE_PIN);
       gpio_output_options_set(RS485_DE_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, RS485_DE_PIN);
	     RS485_DE_ENABLE();
	
	     
	     //���ڳ�ʼ��
	     rcu_periph_clock_enable(RCU_USART0);//ʹ��USART0ʱ��

	     rcu_periph_clock_enable(RCU_GPIOA);
	     gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9);    //���ù���7
       gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);//PA9���óɴ������
       gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_9);
    
	     rcu_periph_clock_enable(RCU_GPIOA);
       gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_10);   //���ù���7
       gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);//PA10���óɴ�������
       gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_10);
	
	     usart_deinit(USART0);    // ���ڸ�λ
       usart_word_length_set(USART0, USART_WL_8BIT);  // �ֳ�
       usart_stop_bit_set(USART0, USART_STB_1BIT);    // ֹͣλ
       usart_parity_config(USART0, USART_PM_NONE);
       usart_baudrate_set(USART0, 9600U);     // ������
       //usart_receive_config(USART1, USART_RECEIVE_ENABLE);        // ����ʹ��
       usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);      // ����ʹ��
       usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
       usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
       usart_enable(USART0);           // ����ʹ��
       //nvic_irq_enable(USART0_IRQn,1,0);
}

/******************************************************************************
*����˵����Rs485���ͺ���
*�������ƣ�RS485_SendData(uint8_t *buf,uint16_t len)
*���������*buf�����������ݻ�����    len�������������ֽ���
*�����������
*����ֵ��  ��
******************************************************************************/
void RS485_SendData(uint8_t *buf,uint16_t len)
{
     uint8_t t;
     //SET_485_TX        // ����ʹ��
     for(t=0;t<len;t++)      
     {           
         while(usart_flag_get(USART0, USART_FLAG_TC) == RESET);  
         usart_data_transmit(USART0,buf[t]);
     }     
    // while(usart_flag_get(USART1, USART_FLAG_TC) == RESET);   
       
         
     //    usart_interrupt_enable(USART1, USART_INT_RBNE);  // �����ж�ʹ��
         //SET_485_RX
}
