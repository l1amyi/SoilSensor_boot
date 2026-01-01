/******************** (C) COPYRIGHT 2022 Lancelot ********************
* File Name          : Interrupt.c
* Author             : Lancelot
* Date First Issued  : 2022-6-18
* Description        : 中断处理函数
********************************************************************************
* History:
22-6-19：GD32F3x0 Demo -- LIB V2.2




*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Interrupt.h"
#include "gd32f30x.h"

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/


/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
    SCB->AIRCR = 0x05FA0004;//复位
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    SCB->AIRCR = 0x05FA0004;//复位
}

/**
  * @brief  this function handles BusFault exception
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    SCB->AIRCR = 0x05FA0004;//复位
}

/**
  * @brief  this function handles BusFault exception
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    SCB->AIRCR = 0x05FA0004;//复位
}

/**
  * @brief  this function handles UsageFault exception
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    SCB->AIRCR = 0x05FA0004;//复位
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
    SCB->AIRCR = 0x05FA0004;//复位
}

/**
  * @brief  this function handles DebugMon exception
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
    SCB->AIRCR = 0x05FA0004;//复位
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
    SCB->AIRCR = 0x05FA0004;//复位
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
