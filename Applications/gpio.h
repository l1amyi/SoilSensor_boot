#ifndef _GPIO_H_
#define _GPIO_H_
#include <stdint.h>

extern uint32_t CHS_PWR_PORT[10];
extern uint32_t CHS_PWR_PIN[10];


void gpio_config(void);

#endif
