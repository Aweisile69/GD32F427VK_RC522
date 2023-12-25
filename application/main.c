/*
 * @author: WangQiWei
 * @Date: 2023-12-18 14:10:01
 * @LastEditors: WangQiWei
 * @LastEditTime: 2023-12-25 11:24:59
 */
#include "gd32f4xx.h"
#include "my_uart.h"
#include "systick.h"
#include "rc522.h"
#include "485Port.h"


int main(void)
{
    /**************************************************************/
    /*使用W25Q64测试SPI通信是否正常,如果正常应能读到MID和DID*/
    // uint8_t MID;
    // uint16_t DID;
    // systick_config();
    // uart_init();
    // HardWare_SPI_Init();
    // HardWare_SPI_Start();
    // HardWare_SPI_SwapByte(0x9F);
    // MID = HardWare_SPI_SwapByte(0xFF);
    // DID = HardWare_SPI_SwapByte(0xFF);
    // DID <<= 8;
    // DID |= HardWare_SPI_SwapByte(0xFF);
    // HardWare_SPI_Stop();
    // uart_TransmitByte(MID);
    // uart_TransmitArray((uint8_t*)&DID,2);
    /**************************************************************/
    systick_config();
    uart_init();
    RC522_Init();
    while(1)
	{
        RC522_Handel();//一直寻卡
    }
}
