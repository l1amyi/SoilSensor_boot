#ifndef _ADC_H_
#define _ADC_H_
#include <stdint.h>

void adc_config(void);
uint16_t adc_read(uint8_t channel);
void adc_off(void);

#endif
