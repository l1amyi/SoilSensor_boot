#ifndef _USART_H_
#define _USART_H_

#include <stdint.h>

void usart0_init(void);
void usart2_init(void);



void uart2_transmit(const uint8_t *buf, int len);
int uart2_receiver(uint8_t *buf, int maxlen, int timeout_0_1ms);

void usart0_dma_receive(uint8_t* buf, int len);

void usart0_transmit(uint8_t* data, int len);

#endif
