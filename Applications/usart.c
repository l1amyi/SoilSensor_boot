#include "usart.h"

#include <string.h>

#include "gd32f30x_usart.h"
#include "gd32f30x_dma.h"

#define RX_LEN 128
#define TX_LEN 128
uint8_t rx_buffer[RX_LEN] = {0};
uint8_t tx_buffer[TX_LEN] = {0};

void usart0_io_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
}

void usart2_io_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);

    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
	gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
}

void usart0_config(void)
{
    rcu_periph_clock_enable(RCU_USART0);

    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);

    usart_interrupt_enable(USART0, USART_INT_IDLE);

    usart_enable(USART0);
    nvic_irq_enable(USART0_IRQn, 0, 14);
    
    usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);
    usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);
}

void usart2_config(void)
{
    rcu_periph_clock_enable(RCU_USART2);

    usart_deinit(USART2);
    usart_baudrate_set(USART2, 9600);
    usart_parity_config(USART2, USART_PM_NONE);
    usart_word_length_set(USART2, USART_WL_8BIT);
    usart_stop_bit_set(USART2, USART_STB_1BIT);
    
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);

    usart_enable(USART2);
}

void usart0_dma_config(void)
{
    dma_parameter_struct dma_init_struct;
    /* enable DMA0 */
    rcu_periph_clock_enable(RCU_DMA0);
    /* deinitialize DMA channel3(USART0 tx) */
    dma_deinit(DMA0, DMA_CH3);
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_addr = 0;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = 0;
    dma_init_struct.periph_addr = ((uint32_t)0x40013804);//USART_DATA(USART0);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(DMA0, DMA_CH3, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH3);
    dma_memory_to_memory_disable(DMA0, DMA_CH3);

    dma_deinit(DMA0, DMA_CH4);
    dma_init_struct.number = RX_LEN;
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)rx_buffer;
    dma_init(DMA0, DMA_CH4, &dma_init_struct);
    /* configure DMA mode */
    dma_circulation_disable(DMA0, DMA_CH4);
    dma_memory_to_memory_disable(DMA0, DMA_CH4);
}

void usart0_init()
{
	usart0_io_config(); // USART0 IO init
	usart0_config(); // USART0 init
    usart0_dma_config();
}

void usart0_dma_receive(uint8_t* buf, int len)
{

    dma_flag_clear(DMA0, DMA_CH4, DMA_FLAG_FTF);

    dma_memory_address_config(DMA0, DMA_CH4, (uint32_t)buf);
    dma_transfer_number_config(DMA0, DMA_CH4, len);
    dma_channel_enable(DMA0, DMA_CH4);
}

void usart2_init()
{
    usart2_io_config();
    usart2_config();
}


void UartPrintf(uint32_t USARTx, uint8_t *bufptr)
{
    uint16_t maxlen = 0;
    gpio_bit_set(GPIOB, GPIO_PIN_12); // set PB12
    while (( *bufptr != '\0' )&&(maxlen < 200))
    {
        usart_data_transmit(USARTx,*bufptr);
        while((usart_flag_get(USARTx, USART_FLAG_TBE) == RESET));
        while((usart_flag_get(USARTx,USART_FLAG_TC)==RESET));
        bufptr++;
        maxlen++;
    }
    gpio_bit_reset(GPIOB, GPIO_PIN_12); // set PB12
}

void uart2_transmit(const uint8_t *buf, int len)
{
    gpio_bit_set(GPIOB, GPIO_PIN_6);
    for (int i = 0; i < len; i++)
    {
        usart_data_transmit(USART2, buf[i]);
        while((usart_flag_get(USART2, USART_FLAG_TBE) == RESET));
        while((usart_flag_get(USART2, USART_FLAG_TC) == RESET));
    }
    gpio_bit_reset(GPIOB, GPIO_PIN_6);
}

extern uint32_t read_time_out_tick;

int uart2_receiver(uint8_t *buf, int maxlen, int timeout_0_1ms)
{
    uint32_t start = read_time_out_tick;
    usart_data_receive(USART2);
    gpio_bit_reset(GPIOB, GPIO_PIN_7);
    while (maxlen > 0)
    {
        if (read_time_out_tick - start > timeout_0_1ms)
        {
            gpio_bit_set(GPIOB, GPIO_PIN_7);
            return 0;
        }
        
        if (usart_flag_get(USART2, USART_FLAG_ORERR) == SET)
        {
            usart_flag_clear(USART2, USART_FLAG_ORERR);
            continue; // Overrun error, skip this byte
        }

        while((usart_flag_get(USART2, USART_FLAG_RBNE) == RESET))
        {
            if (read_time_out_tick - start > timeout_0_1ms)
            {
                gpio_bit_set(GPIOB, GPIO_PIN_7);
                return 0;
            }
        }
        
        *buf = usart_data_receive(USART2);
        
        buf++;
        maxlen--;
    }
    gpio_bit_set(GPIOB, GPIO_PIN_7);
    return 1;
}





void usart0_dma_transmit(uint8_t* data, int len)
{
    if (len > TX_LEN)
    {
        len = TX_LEN;
    }
    
    memcpy(tx_buffer, data, len);
    
    dma_flag_clear(DMA0, DMA_CH3, DMA_FLAG_FTF);
    gpio_bit_set(GPIOB, GPIO_PIN_12);
    dma_transfer_number_config(DMA0, DMA_CH3, len);
    dma_channel_enable(DMA0, DMA_CH3);
    while(dma_flag_get(DMA0, DMA_CH3, DMA_FLAG_FTF) == RESET);
    while(usart_flag_get(USART0, USART_FLAG_TBE) == RESET);
    while(usart_flag_get(USART0,USART_FLAG_TC)==RESET);
    dma_channel_disable(DMA0, DMA_CH3);
    gpio_bit_reset(GPIOB, GPIO_PIN_12);
}

void usart0_transmit(uint8_t* data, int len)
{
    gpio_bit_set(GPIOB, GPIO_PIN_12);
    
    for (int i = 0; i < len; i++)
    {
        usart_data_transmit(USART0, data[i]);
        while((usart_flag_get(USART0, USART_FLAG_TBE) == RESET));
        while((usart_flag_get(USART0, USART_FLAG_TC) == RESET));
    }
    
    gpio_bit_reset(GPIOB, GPIO_PIN_12);
}

#include <stdio.h>


int fputc(int ch, FILE *f)
{
//    usart_data_transmit(USART0, (uint8_t)ch);
//    while((usart_flag_get(USART0, USART_FLAG_TBE) == RESET));
//    while((usart_flag_get(USART0, USART_FLAG_TC) == RESET));
    return ch;
}
