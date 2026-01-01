#include "adc.h"
#include "gd32f30x_adc.h"
#include "gd32f30x_rcu.h"

void adc_gpio_config()
{
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_1);
}

void adc_config()
{
    adc_gpio_config();

    rcu_periph_clock_enable(RCU_ADC0);

    adc_deinit(ADC0);
    
    adc_mode_config(ADC_MODE_FREE);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);

    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL,1);
//    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_1, ADC_SAMPLETIME_239POINT5);

//    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);

//    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
//    
//    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
    
    adc_inserted_channel_config(ADC0, 0, ADC_CHANNEL_1, ADC_SAMPLETIME_239POINT5);
    adc_external_trigger_config(ADC0, ADC_INSERTED_CHANNEL, ENABLE);
    adc_external_trigger_source_config(ADC0, ADC_INSERTED_CHANNEL, ADC0_1_2_EXTTRIG_INSERTED_NONE);

    adc_enable(ADC0);
    
    adc_calibration_enable(ADC0);
}

uint16_t adc_read(uint8_t channel)
{
    if(channel > 1) {
        return 0; // Invalid channel
    }

    //adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
    
    adc_software_trigger_enable(ADC0, ADC_INSERTED_CHANNEL);
    
    while((ADC_STAT(ADC0) & ADC_STAT_EOIC) == 0);
    
    //return adc_regular_data_read(ADC0);
    
    return adc_inserted_data_read(ADC0, ADC_INSERTED_CHANNEL_0);
}

void adc_off()
{
    rcu_periph_clock_disable(RCU_ADC0);
    adc_deinit(ADC0);
    adc_disable(ADC0);
}
