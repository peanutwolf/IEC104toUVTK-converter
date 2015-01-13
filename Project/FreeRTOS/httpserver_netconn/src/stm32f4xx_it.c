/**
  * @file   
  * @author  
  * @version V0.0.1
  * @date    05.03.2014
  * @brief   
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"
#include "stm32f4x7_eth.h"

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* lwip includes */
#include "lwip/sys.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint8_t EthLinkStatus;
extern xSemaphoreHandle s_xSemaphore;
extern xSemaphoreHandle ETH_link_xSemaphore;
extern xSemaphoreHandle xButtonSemaphore;
extern xSemaphoreHandle xSPI_UVTK_Semaphore;
extern xSemaphoreHandle xSPI_UVTK_Mutex;
extern volatile uint8_t SPI_data;
/* Private function prototypes -----------------------------------------------*/
extern void xPortSysTickHandler(void); 
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  xPortSysTickHandler(); 
}

/**
  * @brief  This function handles External line 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  if(EXTI_GetITStatus(ETH_LINK_EXTI_LINE) != RESET)
  {
    /* Give the semaphore to wakeup LwIP task */
    xSemaphoreGiveFromISR(ETH_link_xSemaphore, &xHigherPriorityTaskWoken ); 
  }
  /* Clear interrupt pending bit */
  EXTI_ClearITPendingBit(ETH_LINK_EXTI_LINE);

  /* Switch tasks if necessary. */	
  if( xHigherPriorityTaskWoken != pdFALSE )
  {
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
}

void EXTI0_IRQHandler(void){
	portBASE_TYPE xTaskWoken = pdFALSE;
  if(EXTI_GetITStatus(EXTI_Line0) != RESET)
  {
    if (EthLinkStatus == 0)
    {
			xSemaphoreGiveFromISR( xButtonSemaphore, &xTaskWoken );
       if( xTaskWoken == pdTRUE) {
           taskYIELD();
       }
    }
    /* Clear the EXTI line  pending bit */
    EXTI_ClearITPendingBit(KEY_BUTTON_EXTI_LINE);
  }
}

/**
  * @brief  This function handles ethernet DMA interrupt request.
  * @param  None
  * @retval None
  */
void ETH_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  /* Frame received */
  if ( ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET) 
  {
    /* Give the semaphore to wakeup LwIP task */
    xSemaphoreGiveFromISR( s_xSemaphore, &xHigherPriorityTaskWoken );   
  }

  /* Clear the interrupt flags. */
  /* Clear the Eth DMA Rx IT pending bits */
  ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
  ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);

  /* Switch tasks if necessary. */	
  if( xHigherPriorityTaskWoken != pdFALSE )
  {
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/
/**
  * @brief  Handles SPI3 from UVTK data reception
  * @param  None
  * @retval None
  */
void SPI3_IRQHandler(void)
{
	uint8_t data;
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
  if (SPI_I2S_GetITStatus(SPI3, SPI_I2S_IT_RXNE) == SET) 
  {
     SPI_data = SPI_I2S_ReceiveData(SPI3);
		 //printf("received 0x%08x\n", SPI_data);
     xSemaphoreGiveFromISR(xSPI_UVTK_Semaphore, &xHigherPriorityTaskWoken );   
  }

  if( xHigherPriorityTaskWoken != pdFALSE )
  {
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
