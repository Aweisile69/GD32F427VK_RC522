/*
 * @author: WangQiWei
 * @Date: 2023-12-04 16:37:00
 * @LastEditors: WangQiWei
 * @LastEditTime: 2023-12-20 08:45:19
 */

#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

/* configure systick */
void systick_config(void);
void delay_1us(uint32_t count);
/* delay a time in milliseconds */
void delay_1ms(uint32_t count);
/* delay decrement */
void delay_decrement(void);

#endif /* SYSTICK_H */
