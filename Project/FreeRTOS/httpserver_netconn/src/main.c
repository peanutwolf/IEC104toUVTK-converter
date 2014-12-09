/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "libiecasdu.h"
#include "tcpip.h"
#include "httpserver-netconn.h"
#include "serial_debug.h"
#include "tcp_echoclient.h"
#include "iecsock_server.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

/*--------------- Tasks Priority -------------*/
#define MAIN_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
#define DHCP_TASK_PRIO   ( tskIDLE_PRIORITY + 4 )
#define LED_TASK_PRIO    ( tskIDLE_PRIORITY + 2 )
#define BUT_TASK_PRIO    ( tskIDLE_PRIORITY + 2 )
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern fifo_t* iec_fifo_buf;
extern struct iec_type1 SP_mas[];
xSemaphoreHandle xButtonSemaphore;

static uint8_t* p;
/* Private function prototypes -----------------------------------------------*/
void LCD_LED_Init(void);
void ToggleLed4(void * pvParameters);
void Main_task(void * pvParameters);
void vButtonKeyHandler(void * pvParameters);

/* Private functions ---------------------------------------------------------*/
/*****************************************************
* Debug function for printf();
*****************************************************/
int fputc(int ch, FILE *f) {
  if (DEMCR & TRCENA) {
    while (ITM_Port32(0) == 0);
    ITM_Port8(0) = ch;
  }
  return(ch);
}
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
 {

  /* Configures the priority grouping: 4 bits pre-emption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  
  /* Init task */
  xTaskCreate(Main_task,(int8_t *)"Main", configMINIMAL_STACK_SIZE * 2, NULL,MAIN_TASK_PRIO, NULL);

  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  for( ;; );

}

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
	signed char* name = pcTaskName;
    for(;;){};
}
/**
  * @brief  Main task
  * @param  pvParameters not used
  * @retval None
  */
void Main_task(void * pvParameters)
{
#ifdef SERIAL_DEBUG
  DebugComPort_Init();
#endif

  /*Initialize LCD and Leds */ 
	printf("Initializing system\n");
  LCD_LED_Init();

  /* configure Ethernet (GPIOs, clocks, MAC, DMA) */ 
  ETH_BSP_Config();

  /* Initilaize the LwIP stack */
  LwIP_Init();
	
	/* Initialize RTC */
	InitIEC_RTC();
	
	/* Initialize SPI3 */
	Init_SPI();

  /* Initialize webserver demo */
//  http_server_netconn_init();
		
	iecsock_server_init();
	
	init_IEC104_server();

	UVTK_init_task();
		
	IEC104_init_task();
		 
#ifdef USE_DHCP
  /* Start DHCPClient */
  xTaskCreate(LwIP_DHCP_task, (int8_t *) "DHCP", configMINIMAL_STACK_SIZE * 2, NULL,DHCP_TASK_PRIO, NULL);
#endif

  /* Start toogleLed4 task : Toggle LED4  every 250ms */
  xTaskCreate(ToggleLed4, (int8_t *) "LED4", configMINIMAL_STACK_SIZE, NULL, LED_TASK_PRIO, NULL);
	
	vSemaphoreCreateBinary(xButtonSemaphore);
  xSemaphoreTake( xButtonSemaphore, portMAX_DELAY );	  // Take once just created semaphore
	if( xButtonSemaphore != NULL ) {
      xTaskCreate( &vButtonKeyHandler, (int8_t *) "ButtonHand", configMINIMAL_STACK_SIZE, NULL, BUT_TASK_PRIO, NULL );	
  }
  for( ;; )
  {
      vTaskDelete(NULL);
  }
}


/**
  * @brief  Toggle Led4 task
  * @param  pvParameters not used
  * @retval None
  */
void ToggleLed4(void * pvParameters)
{
  for( ;; )
  {
    /* Toggle LED2 each 250ms */
    STM_EVAL_LEDToggle(LED1);
    vTaskDelay(500);
  }
}

/**
  * @brief  Button Pressed Semaphore Task
  * @param  pvParameters not used
  * @retval None
  */
void vButtonKeyHandler(void * pvParameters)
{
	uint8_t data;
  for( ;; )
  {
     xSemaphoreTake( xButtonSemaphore, portMAX_DELAY );
			if(STM_EVAL_PBGetState(BUTTON_KEY)){
			 // SP_mas[0].sp = 0x01;
				p = pvPortMalloc(1000);
				printf("Heap alloc:%d\n",xPortGetFreeHeapSize());
			}
		  else{
				vPortFree(p);
				printf("Heap free:%d\n",xPortGetFreeHeapSize());
			  //SP_mas[0].sp = 0x00;
//					while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){};
//					 // printf("sending 0x02\n");
//						SPI_I2S_SendData(SPI3, 0x02);
//						vTaskDelay(1);
//				  while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){};
//					  //printf("sending 0x19\n");
//						SPI_I2S_SendData(SPI3, 0x19); 
//					  vTaskDelay(1);
//				  while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){};
//					  //printf("sending 0x24\n");
//						SPI_I2S_SendData(SPI3, 0x24);
//					  vTaskDelay(1);
//				  while(SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_BSY) == SET){};
//					  //printf("sending 0x0C\n");
//						SPI_I2S_SendData(SPI3, 0x0C);
//					  vTaskDelay(1);
//					  STM_EVAL_LEDToggle(LED1);
			}
			//fifoPushElem(iec_fifo_buf, prepare_data_iframe(SPON_COT, M_SP_TB_1, 0x00, 0x01));
   }
}


/**
  * @brief  Initializes the STM324xG-EVAL's LCD and LEDs resources.
  * @param  None
  * @retval None
  */
void LCD_LED_Init(void)
{

  /* Initialize STM324xG-EVAL's LEDs */
  STM_EVAL_LEDInit(LED1);
  STM_EVAL_LEDInit(LED2);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);

STM_EVAL_PBInit(BUTTON_KEY, BUTTON_MODE_EXTI);
}

/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of Ticks to delay.
  * @retval None
  */
void Delay(uint32_t nCount)
{
  vTaskDelay(nCount);
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}


#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
