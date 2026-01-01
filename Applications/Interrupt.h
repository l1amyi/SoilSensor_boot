

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __INTERRUPT_H
#define __INTERRUPT_H


/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


void NMI_Handler(void);                     /* this function handles NMI exception */
void HardFault_Handler(void);               /* this function handles HardFault exception */
void MemManage_Handler(void);               /* this function handles MemManage exception */
void BusFault_Handler(void);                /* this function handles BusFault exception */
void UsageFault_Handler(void);              /* this function handles UsageFault exception */
void SVC_Handler(void);                     /* this function handles SVC exception */
void DebugMon_Handler(void);                /* this function handles DebugMon exception */
void PendSV_Handler(void);                  /* this function handles PendSV exception */


//void EXTI0_1_IRQHandler(void);
//void EXTI2_3_IRQHandler(void);
//void EXTI4_15_IRQHandler(void);

//void TIM1_BRK_UP_TRG_COM_IRQHandler(void);
   
void TIMER13_IRQHandler(void);     
void TIMER14_IRQHandler(void);     
void TIMER15_IRQHandler(void);   
void TIMER16_IRQHandler(void);     

void USART0_IRQHandler(void);

/*void PPP_IRQHandler(void);*/


#endif 
/***********************end******************************/

