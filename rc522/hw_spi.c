/*
 * @author: WangQiWei
 * @Date: 2023-12-05 23:51:17
 * @LastEditors: WangQiWei
 * @LastEditTime: 2023-12-24 17:33:25
 */
#include "gd32f4xx.h"
#include "hw_spi.h"

/**
 * @description: RC522的SPI初始化
 * @return {*}
 */
void HardWare_SPI_Init(void)
{
    /*开启GPIO和SPI的时钟*/
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOE);
    rcu_periph_clock_enable(RCU_SPI0);
    /*配置GPIO口*/
    /*CS*/
    gpio_mode_set(CS_PORT,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,CS_PIN);
    gpio_output_options_set(CS_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,CS_PIN);
    /*SCK*/
    gpio_af_set(SCK_PORT,GPIO_AF_5,SCK_PIN);
    gpio_mode_set(SCK_PORT,GPIO_MODE_AF,GPIO_PUPD_NONE,SCK_PIN);
    gpio_output_options_set(SCK_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,SCK_PIN);
    /*MOSI*/
    gpio_af_set(MOSI_PORT,GPIO_AF_5,MOSI_PIN);
    gpio_mode_set(MOSI_PORT,GPIO_MODE_AF,GPIO_PUPD_NONE,MOSI_PIN);
    gpio_output_options_set(MOSI_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,MOSI_PIN);
    /*MISO*/
    gpio_af_set(MISO_PORT,GPIO_AF_5,MISO_PIN);
    gpio_mode_set(MISO_PORT,GPIO_MODE_AF,GPIO_PUPD_NONE,MISO_PIN);
    gpio_output_options_set(MISO_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,MISO_PIN);
    /*RESET*/
    gpio_mode_set(RESET_PORT,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,RESET_PIN);
    gpio_output_options_set(RESET_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,RESET_PIN);
    /*配置SPI*/
    spi_parameter_struct spi_init_struct;
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss  = SPI_NSS_SOFT;
    spi_init_struct.prescale = SPI_PSC_64;    //注意这里的预分频系数，如果通信有问题，首先把每个预分频系数都试一遍
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    /*初始化SPI*/
    spi_init(SPI0, &spi_init_struct);
    //使能SPI
    spi_enable(SPI0);
    /*置CS为高电平，默认不选中任何从机*/
    CS_Disable();
}


/**
 * @description: 硬件SPI交换一个字节数据
 * @param {uint8_t} byte 数据
 * @return {*} 交换后，得到的一个字节数据
 */
uint8_t HardWare_SPI_SwapByte(uint8_t byte) 
{
    // 等待发送缓冲区为空
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
    // 发送一个字节
    spi_i2s_data_transmit(SPI0, byte);
    // 等待接收缓冲区非空
    while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));
    // 读取一个字节
    return spi_i2s_data_receive(SPI0);
}


/**
 * @description: 硬件SPI开始通信
 * @return {*}
 */
void HardWare_SPI_Start(void)
{
    CS_Enable();
}

/**
 * @description: 硬件SPI结束通信
 * @return {*}
 */
void HardWare_SPI_Stop(void)
{
    CS_Disable();
}