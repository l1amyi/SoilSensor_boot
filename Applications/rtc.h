#ifndef _RTC_H_
#define _RTC_H_
#include <stdint.h>

void RTC_IRQHandler(void);
void rtc_configuration(void);
void rtc_config(void);

void rtc_setval(uint32_t val);
uint32_t rtc_getval(void);

#endif
