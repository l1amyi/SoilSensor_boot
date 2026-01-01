#include "rtc.h"

#include "gd32f30x_rtc.h"
#include "gd32f30x_rcu.h"



void RTC_IRQHandler(void)
{
    if(rtc_flag_get(RTC_FLAG_SECOND) != RESET){
        rtc_flag_clear(RTC_FLAG_SECOND);

    }
    if(rtc_flag_get(RTC_FLAG_ALARM) != RESET){
        /* clear the RTC alarm interrupt flag*/
        rtc_flag_clear(RTC_FLAG_ALARM); 
    }
}

void rtc_configuration(void)
{
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
    bkp_deinit();

    /* enable HXTAL */
    rcu_osci_on(RCU_HXTAL);
    /* wait till HXTAL is ready */
    rcu_osci_stab_wait(RCU_HXTAL);
    
    /* select RCU_HXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_HXTAL_DIV_128);

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_SECOND);
    rtc_interrupt_enable(RTC_INT_ALARM);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set RTC prescaler: set RTC period to 1s */
    rtc_prescaler_set(62500);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

void rtc_config()
{
    nvic_irq_enable(RTC_IRQn, 1, 0);
    
    uint32_t RTCSRC_FLAG = GET_BITS(RCU_BDCTL, 8, 9);
    
    
    if ((0xA5A5 != bkp_read_data(BKP_DATA_0)) || (0x00 == RTCSRC_FLAG)){
        /* RTC configuration */
        rtc_configuration();

        uint32_t temp = 0;
        /* wait until last write operation on RTC registers has finished */
        rtc_lwoff_wait();
        temp = 0;
        /* change the current time */
        rtc_counter_set(temp);
        rtc_lwoff_wait();
        //rtc_alarm_config((temp + 10) % 0x00015180);
        //rtc_lwoff_wait();

        bkp_write_data(BKP_DATA_0, 0xA5A5);
    }else{
        rcu_osci_on(RCU_HXTAL);
        rcu_osci_stab_wait(RCU_HXTAL);
        
        rcu_periph_clock_enable(RCU_PMU);
        pmu_backup_write_enable();
        

        rtc_register_sync_wait();
        
        rcu_rtc_clock_config(RCU_RTCSRC_HXTAL_DIV_128);
        rtc_prescaler_set(62500);
        
        rtc_lwoff_wait();
        rtc_interrupt_enable(RTC_INT_SECOND);
        rtc_interrupt_enable(RTC_INT_ALARM);
        rtc_lwoff_wait();
        
        bkp_write_data(BKP_DATA_0, 0xA5A5);
    }
}

void rtc_setval(uint32_t val)
{
    rtc_lwoff_wait();
    rtc_counter_set(val);
    rtc_lwoff_wait();
}

uint32_t rtc_getval()
{
    return rtc_counter_get();
}
