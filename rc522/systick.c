/*
 * @author: WangQiWei
 * @Date: 2023-12-04 16:37:00
 * @LastEditors: WangQiWei
 * @LastEditTime: 2023-12-20 08:46:50
 */

#include "gd32f4xx.h"
#include "systick.h"

volatile static uint32_t delay;

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void systick_config(void)
{
    /* setup systick timer for 1MHz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000000U)){
        /* capture error */
        while(1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1us(uint32_t count)
{
    delay = count;
    while(0U != delay){
    }
}

void delay_1ms(uint32_t count)
{
    delay_1us(count*1000U);
}

/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)
{
    if(0U != delay){
        delay--;
    }
}
