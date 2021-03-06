/**
  * @file   
  * @author  
  * @version V0.0.1
  * @date    05.03.2014
  * @brief   
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "string.h"
#include "httpserver-netconn.h"
#include "FreeRTOS.h"
#include "task.h"
#include "STM324x7i_eval.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define WEBSERVER_THREAD_PRIO    ( tskIDLE_PRIORITY + 5 )

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern const unsigned char start_page[915];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief serve tcp connection  
  * @param conn: pointer on connection structure 
  * @retval None
  */
		char* buf;
		  u16_t buflen;
void http_server_serve(struct netconn *conn) 
{
  struct netbuf *inbuf;
  err_t recv_err;



  recv_err = netconn_recv(conn, &inbuf);
	if (recv_err == ERR_OK){
		netbuf_data(inbuf, (void**)&buf, &buflen);
		
		printf("%s\n", buf);
		 if ((buflen >=5) && (strncmp(buf, "POST /", 5) == 0)){
			 printf("%s\n",strstr(buf, "gw3"));
				printf("in POST\n");
				//netconn_write(conn, (const unsigned char*)(start_page), (size_t)get_start_page_size(), NETCONN_NOCOPY);
		 }
		 else if ((buflen >=5) && (strncmp(buf, "GET /", 5) == 0)){
				printf("in GET\n");
				netconn_write(conn, (const unsigned char*)(start_page), (size_t)get_start_page_size(), NETCONN_NOCOPY);
		 }
	}
	STM_EVAL_LEDOn(LED2);
	//for(;;);

  netconn_close(conn);

  netbuf_delete(inbuf);
}


/**
  * @brief  http server thread 
  * @param arg: pointer on argument(not used here) 
  * @retval None
  */
static void http_server_netconn_thread(void *arg)
{ 
  struct netconn *conn, *newconn;
  err_t err, accept_err;
  
  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);
  if (conn!= NULL)
  {
    /* Bind to port 80 (HTTP) with default IP address */
    err = netconn_bind(conn, NULL, 80);
    
    if (err == ERR_OK)
    {
      /* Put the connection into LISTEN state */
      netconn_listen(conn);
  
      while(1) 
      {
        /* accept any icoming connection */
        accept_err = netconn_accept(conn, &newconn);
        if(accept_err == ERR_OK)
        {
          /* serve connection */
          http_server_serve(newconn);

          /* delete connection */
          netconn_delete(newconn);
        }
      }
    }
    else
    {
      /* delete connection */
      netconn_delete(newconn);
      printf("can not bind netconn");
    }
  }
  else
  {
    printf("can not create netconn");
  }
}

/**
  * @brief  Initialize the HTTP server (start its thread) 
  * @param  none
  * @retval None
  */
void http_server_netconn_init()
{
  sys_thread_new("HTTP", http_server_netconn_thread, NULL, DEFAULT_THREAD_STACKSIZE, WEBSERVER_THREAD_PRIO);
}

