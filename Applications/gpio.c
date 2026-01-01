#include "gpio.h"

#include "gd32f30x_gpio.h"


uint32_t CHS_PWR_PORT[10] = {GPIOB, GPIOB, GPIOB, GPIOD, GPIOC, 
                              GPIOC, GPIOC, GPIOA, GPIOA, GPIOA};
uint32_t CHS_PWR_PIN[10] = {GPIO_PIN_5, GPIO_PIN_4, GPIO_PIN_3, GPIO_PIN_2, GPIO_PIN_12,
                          GPIO_PIN_11, GPIO_PIN_10, GPIO_PIN_15, GPIO_PIN_12, GPIO_PIN_11};

//uint32_t CHS_PWR_PORT[10] = {GPIOA, GPIOC, GPIOB, GPIOB, GPIOC,
//                             GPIOA, GPIOA, GPIOC, GPIOD, GPIOB};

//uint32_t CHS_PWR_PIN[10] = {GPIO_PIN_12, GPIO_PIN_10, GPIO_PIN_5, GPIO_PIN_3, GPIO_PIN_12,
//                            GPIO_PIN_11, GPIO_PIN_15, GPIO_PIN_11, GPIO_PIN_2, GPIO_PIN_4};

void gpio_config()
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_AF);
  

    
	// PB0 state
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
	gpio_bit_set(GPIOB, GPIO_PIN_0); // set PB0
    
    // PB12 DRE
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_bit_reset(GPIOB, GPIO_PIN_12); // reset DRE
    
    // PB6 DE
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    gpio_bit_reset(GPIOB, GPIO_PIN_6); // reset PB6
    
//    // PB10 test
//    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
//    gpio_bit_set(GPIOB, GPIO_PIN_10);

    // PB7 RE
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    gpio_bit_set(GPIOB, GPIO_PIN_7); // reset PB7

    // PC3 PWR ALL
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    gpio_bit_reset(GPIOC, GPIO_PIN_3); // reset PC3

    // remap  
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    //PA12 CHx_PWR
    for (int i = 0; i < 10; i++)
    {
        gpio_init(CHS_PWR_PORT[i], GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, CHS_PWR_PIN[i]);
        gpio_bit_reset(CHS_PWR_PORT[i], CHS_PWR_PIN[i]); // reset CHx_PWR
    }

    // SPI0 CS0 PA4
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_bit_set(GPIOA, GPIO_PIN_4); // reset PA4
    
    // SPI0 CS1 PC5
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    gpio_bit_set(GPIOC, GPIO_PIN_5); // reset PC5
    
    // PA0 ChargeDet
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
    
    // PC13
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_bit_set(GPIOC, GPIO_PIN_13);
    
    
    
    
    
    
}
