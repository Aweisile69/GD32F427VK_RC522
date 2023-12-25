/*
 * @author: WangQiWei
 * @Date: 2023-12-18 14:10:01
 * @LastEditors: WangQiWei
 * @LastEditTime: 2023-12-25 10:17:56
 */
#include "gd32f4xx.h"
#include "my_uart.h"
#include "systick.h"
#include "rc522.h"
#include "485Port.h"


int main(void)
{
    systick_config();
    uart_init();
    RC522_Init();
    while(1)
	{
        RC522_Handel();//一直寻卡
    }
}
