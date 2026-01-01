/******************** (C) COPYRIGHT 2022 Lancelot ********************
* File Name          : main.c
* Author             : Lancelot
* Date First Issued  : 2022-6-18
* Description        : Main program body
********************************************************************************
* History:
GD32F3x0 Demo -- LIB V2.2




*******************************************************************************/

// /* Local includes ------------------------------------------------------------*/
#include "main.h"
#include "gd32f30x_fmc.h"
#include "gd32f30x.h"
#include "systick.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

///////////////////////////////////////////////////////////////////
#define     FLASH_PAGE_SIZE         (0x800)        //2048  2K 
#define     DEVICE_ID_ADDR          0x0800F800

#define     APPLICATION_ADDRESS     0x08001000          //APP程序首地址
#define     UPDATE_DATA_ADDR        0x08010000
#define     UPDATE_FLAG_ADDR        0x0801FFFC          //升级标志地址
#define     CODE_LEN                30              // 30 pages

#define     VAR_ADDR                0x0801F800

#define     FLASH_INIT_FLAG         0x8F7f6f5f
#define     FLASH_CODE_FLAG         0x379C6B5D

#define     UPDATE_FLAG             0x7F7F7F7F
////////

void RCC_Configuration(void);

static void JumpToApp(void);


int main(void)
{
    RCC_Configuration();
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    systick_config();
    
    uint32_t tempi;
    fmc_state_enum FLASHStatus;
    uint32_t UpdateFlag = *(uint32_t*)UPDATE_FLAG_ADDR;
    
    if(UpdateFlag == UPDATE_FLAG)
    {
        for (int page_index = 0; page_index < CODE_LEN; page_index++)
        {
            ReWrite:
            fmc_unlock();
            /* clear all pending flags */
            fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
            FLASHStatus = fmc_page_erase(APPLICATION_ADDRESS + page_index * FLASH_PAGE_SIZE);
            tempi = 100000;
            while((FLASHStatus != FMC_READY)&&(tempi>0))
            {
                tempi--;
            }

            /* Clear All pending flags */
            fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
            
            for(int byte_index = 0; byte_index < FLASH_PAGE_SIZE; byte_index += 4)
            {
                uint32_t WAddress = APPLICATION_ADDRESS + page_index * FLASH_PAGE_SIZE + byte_index;
                uint32_t RAddress = UPDATE_DATA_ADDR + page_index * FLASH_PAGE_SIZE + byte_index;
                uint32_t Data = *(volatile uint32_t*)RAddress;
                FLASHStatus = fmc_word_program(WAddress, Data);
                fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
                uint32_t TempDataW = *(volatile uint32_t*)WAddress;
                if(TempDataW != Data)
                {
                    goto ReWrite;
                }
            }
        }
        
        ReEraseFlag:
        /* unlock the flash program/erase controller */
        fmc_unlock();
        /* clear all pending flags */
        fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
        /* Erase the FLASH pages */
        FLASHStatus = fmc_page_erase(VAR_ADDR); 
        tempi = 100000;
        while((FLASHStatus != FMC_READY)&&(tempi>0))
        {
            tempi--;
        }    
        /* Clear All pending flags */
        fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR);
        
        UpdateFlag = *(uint32_t*)UPDATE_FLAG_ADDR;
        
        if(UpdateFlag == UPDATE_FLAG)
        {
            goto ReEraseFlag;
        }
        fmc_lock();
    }
    JumpToApp();
}

#include "gd32f30x_rcu.h"

void RCC_Configuration(void) 
{
// 120 MHz
//    rcu_pllpresel_config(RCU_PLLPRESRC_HXTAL);
//    rcu_predv0_config(RCU_PREDV0_DIV1);
//  
//    rcu_pll_config(RCU_PLLSRC_HXTAL_IRC48M, RCU_PLL_MUL15);
//  
//    rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);
//    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
//  
//    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV2);
//  
//    rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);
//  
//    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV4);
  

// 8Mhz
//    rcu_pllpresel_config(RCU_PLLPRESRC_HXTAL);
//    rcu_predv0_config(RCU_PREDV0_DIV2);
//  
//    rcu_pll_config(RCU_PLLSRC_HXTAL_IRC48M, RCU_PLL_MUL2);
//  
//    rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);
//    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
//  
//    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV1);
//  
//    rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);
//  
//    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV2);

// 8Mhz inner
//    rcu_system_clock_source_config(RCU_CKSYSSRC_IRC8M);
//    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
//  
//    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV1);
//  
//    rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);
//  
//    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV2);    
    
    
// 8Mhz outer
    rcu_system_clock_source_config(RCU_CKSYSSRC_HXTAL);
    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
  
    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV1);
  
    rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);
  
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV2);    

}

typedef  void (*pFunction)(void);
pFunction   JumpToApplication;
static uint32_t JumpAddress;
static void JumpToApp(void)
{
    if (((*(volatile uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
    { 
      /* Jump to user application */
      JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
      JumpToApplication = (pFunction) JumpAddress;
      /* Initialize user application's Stack Pointer */
      __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
      JumpToApplication();
    }
}

//////////////////////////////////////////


