/**
  * @file    main.c
  * @author  Vigurskiy.ES
  * @version V0.0.1
  * @date    05.03.2014
  * @brief   Main program body
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


/* Private define ------------------------------------------------------------*/
/* Defines to push debug info to STLink  -> see fputc(int ch, FILE *f)*/
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

/*--------------- Tasks Priority -------------*/
#define MAIN_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
#define LED_TASK_PRIO    ( tskIDLE_PRIORITY + 2 )
#define BUT_TASK_PRIO    ( tskIDLE_PRIORITY + 2 )

/* Private variables ---------------------------------------------------------*/
extern fifo_t* iec_fifo_buf;
xSemaphoreHandle xButtonSemaphore;
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
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  xTaskCreate(Main_task,(int8_t *)"Main", configMINIMAL_STACK_SIZE * 2, NULL,MAIN_TASK_PRIO, NULL);
  vTaskStartScheduler();
  for( ;; );
}

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
		signed char* name = pcTaskName;
    for(;;){};
}
/**
  * @brief  Main task. Initialize system modules
  * @param  pvParameters not used
  * @retval None
  */
void Main_task(void * pvParameters)
{
#ifdef SERIAL_DEBUG
  DebugComPort_Init();
#endif
	printf("Initializing system\n");
	
	Vars_Init();
	
  LCD_LED_Init();

  /* configure Ethernet physical layer RMII */ 
  ETH_BSP_Config();

  /* Initilaize the LwIP stack */
  LwIP_Init();
	
	/* Initialize RTC */
	InitIEC_RTC();
	
	/* Initialize SPI3 */
	Init_SPI();
	
	/* Initialize Independent Watchdog */
	Init_IWDT();

  /* Initialize webserver over netconn API */
	/* TODO: Make websrv parse GET req*/
  http_server_netconn_init();
		
	/* Initialize TCP/IP layer for IEC_104 */	
	iecsock_server_init();
	
	/* Initialize IEC_104 layer */
	init_IEC104_server();

	/* Initialize UVTK board poll over SPI3 */
	UVTK_init_task();
	
	/* Initialize IEC104 data poll  */
	IEC104_init_task();
		 

  /* Initialize LED4 toggle and UserButton handler for STM32F4Discovery*/
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
  * @brief  Toggle Led4 task, Reload IWDG
  * @param  pvParameters not used
  * @retval None
  */
void ToggleLed4(void * pvParameters)
{
  for( ;; )
  {
    STM_EVAL_LEDToggle(LED1);
		IWDG_ReloadCounter();
    vTaskDelay(500);
  }
}

void Vars_Init(void){
		if(*(uint8_t*)SETTINGS_ADDRESS != 0xFE){
				Reset_Device_Default();
		}
}

/**
  * @brief  Button Pressed Semaphore Task
  * @param  pvParameters not used
  * @retval None
  */
void vButtonKeyHandler(void * pvParameters)
{
  for( ;; )
  {
     xSemaphoreTake( xButtonSemaphore, portMAX_DELAY );
		 vTaskDelay(50);
		 if(STM_EVAL_PBGetState(BUTTON_KEY)){
				Reset_Device_Default();
				STM_EVAL_LEDToggle(LED4);
		 }
   }
}


/**
  * @brief  Initializes the STM32F4Discovery LEDs resources.
  * @param  None
  * @retval None
  */
void LCD_LED_Init(void)
{
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
  while (1)
  {}
}


#endif

