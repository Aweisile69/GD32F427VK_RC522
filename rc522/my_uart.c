/*
 * @author: qiwei.wang
 * @Date: 2023-11-27 14:26:03
 * @LastEditors: WangQiWei
 * @LastEditTime: 2023-12-21 15:59:23
 */
#include "gd32f4xx.h"
#include "my_uart.h"
#include "stdio.h"
#include "string.h"
#include <stdarg.h>


/**
 * @description: 串口初始化
 * PB6->USART0_TX
 * PB7->USART0_RX
 * @return {*}
 */
void uart_init(void) 
{
    /* 开启GPIOB和串口时钟 */
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_USART0);
    /* PB6和PB7复用为串口 */
    gpio_af_set(GPIOB, GPIO_AF_7, GPIO_PIN_6);
    gpio_af_set(GPIOB, GPIO_AF_7, GPIO_PIN_7);
    /* 配置GPIO为复用推挽输出*/
    gpio_mode_set(GPIOB,GPIO_MODE_AF,GPIO_PUPD_PULLUP,GPIO_PIN_6);
    gpio_mode_set(GPIOB,GPIO_MODE_AF,GPIO_PUPD_PULLUP,GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    /* 复位USART0 */
    usart_deinit(USART0);
    /* 设置串口波特率*/
    usart_baudrate_set(USART0, 9600U);
    /* 设置传输数据的长度*/
    usart_word_length_set(USART0, USART_WL_8BIT);
    /* 设置停止位的长度*/
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    /* 设置奇偶校验位*/
    usart_parity_config(USART0, USART_PM_NONE);
    /* 关闭硬件流控制*/
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    /* 使能串口接收和发送*/
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    /* 使能串口 */
    usart_enable(USART0);
}


/**
 * @description: 发送一个字节
 * @param {uint8_t} byte
 * @return {*}
 */
void uart_TransmitByte(uint8_t byte)
{
    //发送一个字节，不会出现第二个字节吞掉第一个的情况，所以这里不需要先读SR寄存器
    //while(usart_flag_get(USART_PERIPHERAL,USART_FLAG_TC) == RESET);
    //把一个字节数据写入USART_PERIPHERAL的数据寄存器
    usart_data_transmit(USART0,byte);
    /*等待数据发送完毕，这里需要判断的标志位是USART_FLAG_TC而不是USART_FLAG_TBE
    看数据手册*/
    while(usart_flag_get(USART0, USART_FLAG_TC) == RESET);
}

/**
 * @description: 发送数组
 * @param {uint8_t} *array
 * @param {uint16_t} length
 * @return {*}
 */
void uart_TransmitArray(uint8_t *array,uint16_t length)
{
    //进行一次读SR寄存器的操作，和第一次循环写DR寄存器搭配，软件清零TC标志位
    while(usart_flag_get(USART0,USART_FLAG_TC) == RESET);
    for (uint16_t i = 0; i < length; i++)
    {
        uart_TransmitByte(array[i]);
    }
}

/**
 * @description: 发送字符串
 * @param {char} *string
 * @return {*}
 */
void uart_TransmitString(char *string)
{
    while(usart_flag_get(USART0,USART_FLAG_TC) == RESET);
    /*字符串的最后一个字符为'/0'也就是0，循环的终止条件就是*string*/
    while (*string)
    {
        usart_data_transmit(USART0, *string);
        while (usart_flag_get(USART0, USART_FLAG_TC) == RESET);
        string++;
    }
}


/**
  * 函    数：次方函数（内部使用）
  * 返 回 值：返回值等于X的Y次方
  */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//设置结果初值为1
	while (Y --)			//执行Y次
	{
		Result *= X;		//将X累乘到结果
	}
	return Result;
}

void uart_TransmitNumber(uint32_t number,uint8_t length)
{
    uint8_t i;
	for (i = 0; i < length; i ++)		//根据数字长度遍历数字的每一位
	{
		uart_TransmitByte(number / Serial_Pow(10, length - i - 1) % 10 + '0');	//依次调用Serial_SendByte发送每位数字
	}
}

/**
  * 函    数：使用printf需要重定向的底层函数
  * 参    数：保持原始格式即可，无需变动
  * 返 回 值：保持原始格式即可，无需变动
  */
int fputc(int ch, FILE *f)
{
	uart_TransmitByte(ch);			//将printf的底层重定向到自己的发送字节函数
	return ch;
}

/**
  * 函    数：自己封装的prinf函数
  * 参    数：format 格式化字符串
  * 参    数：... 可变的参数列表
  * 返 回 值：无
  */
void uart_printf(char *format, ...)
{
	char String[100];				//定义字符数组
	va_list arg;					//定义可变参数列表数据类型的变量arg
	va_start(arg, format);			//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);	//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);					//结束变量arg
	uart_TransmitString(String);		//串口发送字符数组（字符串）
}


