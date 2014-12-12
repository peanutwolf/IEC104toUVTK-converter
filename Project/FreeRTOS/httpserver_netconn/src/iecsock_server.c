
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "IEC104_server.h"

#define IECSOCK_PORT 2405

#if LWIP_TCP

static struct tcp_pcb *iecsock_tcp_pcb;
extern volatile u8_t NS, NR; 
extern fifo_t* iec_fifo_buf;
/*Protocol states */
enum tcp_iecsock_states
{
  IEC_NONE = 0,
  IEC_ACCEPTED,
  IEC_RECEIVED,
  IEC_CLOSING
};

/* structure for maintaing connection infos to be passed as argument 
   to LwIP callbacks*/
struct iecsock_struct
{
  u8_t state;             /* current connection state */
  struct tcp_pcb *pcb;    /* pointer on the current tcp_pcb */
  struct pbuf *p;         /* pointer on the received/to be transmitted pbuf */
};


static err_t iecsock_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t iecsock_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void iecsock_server_error(void *arg, err_t err);
static err_t iecsock_server_poll(void *arg, struct tcp_pcb *tpcb);
static err_t iecsock_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void iecsock_server_send(struct tcp_pcb *tpcb, struct iecsock_struct *iec_struct);
static void iecsock_server_connection_close(struct tcp_pcb *tpcb, struct iecsock_struct *iec_struct);


/**
  * @brief  Initializes the tcp echo server
  * @param  None
  * @retval None
  */
void iecsock_server_init(void)
{
  /* create new tcp pcb */
  iecsock_tcp_pcb = tcp_new();

  if (iecsock_tcp_pcb != NULL)
  {
    err_t err;
    
    /* bind echo_pcb to port IECSOCK_PORT */
    err = tcp_bind(iecsock_tcp_pcb, IP_ADDR_ANY, IECSOCK_PORT);
    
    if (err == ERR_OK)
    {
      /* start tcp listening for echo_pcb */
      iecsock_tcp_pcb = tcp_listen(iecsock_tcp_pcb);
      
      /* initialize LwIP tcp_accept callback function */
      tcp_accept(iecsock_tcp_pcb, iecsock_server_accept);	
    }
    else 
    {
      /* deallocate the pcb */
      memp_free(MEMP_TCP_PCB, iecsock_tcp_pcb);
      printf("Can not bind pcb\n");
    }
  }
  else
  {
    printf("Can not create new pcb\n");
  }
}

/**
  * @brief  This function is the implementation of tcp_accept LwIP callback
  * @param  arg: not used
  * @param  newpcb: pointer on tcp_pcb struct for the newly created tcp connection
  * @param  err: not used 
  * @retval err_t: error status
  */
static err_t iecsock_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  err_t ret_err;
  struct iecsock_struct *iec_struct;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  /* set priority for the newly accepted tcp connection newpcb */
  tcp_setprio(newpcb, TCP_PRIO_MIN);

  /* allocate structure es to maintain tcp connection informations */
  iec_struct = (struct iecsock_struct*)pvPortMalloc(sizeof(struct iecsock_struct));
  if (iec_struct != NULL)
  {
    iec_struct->state = IEC_ACCEPTED;
    iec_struct->pcb = newpcb;
    iec_struct->p = NULL;
    
    /* pass newly allocated es structure as argument to newpcb */
    tcp_arg(newpcb, iec_struct);
    
    /* initialize lwip tcp_recv callback function for newpcb  */ 
    tcp_recv(newpcb, iecsock_server_recv);
    
    /* initialize lwip tcp_err callback function for newpcb  */
    tcp_err(newpcb, iecsock_server_error);
    
    /* initialize lwip tcp_poll callback function for newpcb */
    tcp_poll(newpcb, iecsock_server_poll, 1);
    
    ret_err = ERR_OK;
  }
  else
  {
    /*  close tcp connection */
    iecsock_server_connection_close(newpcb, iec_struct);
    /* return memory error */
    ret_err = ERR_MEM;
  }
  return ret_err;  
}


/**
  * @brief  This function is the implementation for tcp_recv LwIP callback
  * @param  arg: pointer on a argument for the tcp_pcb connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  pbuf: pointer on the received pbuf
  * @param  err: error information regarding the reveived pbuf
  * @retval err_t: error code
  */
static err_t iecsock_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct iecsock_struct *iec_struct;
  err_t ret_err;
	u8_t start;

  LWIP_ASSERT("arg != NULL",arg != NULL);
  
  iec_struct = (struct iecsock_struct*)arg;
	
	start = *(u8_t*)p -> payload;
	if (p == NULL)
  {
    /* remote host closed connection */
    iec_struct->state = IEC_CLOSING;
    if(iec_struct->p == NULL)
    {
       /* we're done sending, close connection */
       iecsock_server_connection_close(tpcb, iec_struct);
    }
    else
    {
      /* we're not done yet */
      /* acknowledge received packet */
      tcp_sent(tpcb, iecsock_server_sent);
      
      /* send remaining data*/
      iecsock_server_send(tpcb, iec_struct);
    }
    ret_err = ERR_OK;
  }   
  /* else : a non empty frame was received from client but for some reason err != ERR_OK */
  else if(err != ERR_OK)
  {
    /* free received pbuf*/
    iec_struct->p = NULL;
    pbuf_free(p);
    ret_err = err;
  }
  else if(iec_struct->state == IEC_ACCEPTED)
  {
    /* first data chunk in p->payload */
    iec_struct->state = IEC_RECEIVED;
		if(start == 0x68){
		  p = generatIECansw(p);
			iec_struct->p = p;	
			if(iec_struct->p != NULL){
				iecsock_server_send(iec_struct->pcb, iec_struct);
			}
		}    
		else{
			pbuf_free(p);
		}
    /* initialize LwIP tcp_sent callback function */
    tcp_sent(tpcb, iecsock_server_sent);
		
    ret_err = ERR_OK;
  }
  else if (iec_struct->state == IEC_RECEIVED)
  {
    /* more data received from client and previous data has been already sent*/
    if(iec_struct->p == NULL)
    {   
		  if(start == 0x68){
		     p = generatIECansw(p);
				 iec_struct->p = p;
				 if(iec_struct->p != NULL){
					 iecsock_server_send(iec_struct->pcb, iec_struct);
				 }
		  }
      else{
				 pbuf_free(p);
			}
    }
    else
    {
      struct pbuf *ptr;
      /* chain pbufs to the end of what we recv'ed previously  */
      ptr = iec_struct->p;
      pbuf_chain(ptr,p);
    }
    ret_err = ERR_OK;
  }
  
  /* data received when connection already closed */
  else
  {
    /* Acknowledge data reception */
    tcp_recved(tpcb, p->tot_len);
    
    /* free pbuf and do nothing */
    iec_struct->p = NULL;
    pbuf_free(p);
    ret_err = ERR_OK;
  }
  return ret_err;
}

/**
  * @brief  This function implements the tcp_err callback function (called
  *         when a fatal tcp_connection error occurs. 
  * @param  arg: pointer on argument parameter 
  * @param  err: not used
  * @retval None
  */
static void iecsock_server_error(void *arg, err_t err)
{
  struct iecsock_struct *iec_struct;

  LWIP_UNUSED_ARG(err);

  iec_struct = (struct iecsock_struct *)arg;
  if (iec_struct != NULL)
  {
    /*  free es structure */
    vPortFree(iec_struct);
  }
}

/**
  * @brief  This function implements the tcp_poll LwIP callback function
  * @param  arg: pointer on argument passed to callback
  * @param  tpcb: pointer on the tcp_pcb for the current tcp connection
  * @retval err_t: error code
  */
static err_t iecsock_server_poll(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  struct iecsock_struct *iec_struct;

  iec_struct = (struct iecsock_struct *)arg;
  if (iec_struct != NULL)
  {
		iec_struct->p = prepare_tcp_iec_buf(iec_fifo_buf);
    if (iec_struct->p != NULL)
    {
      /* there is a remaining pbuf (chain) , try to send data */
      iecsock_server_send(tpcb, iec_struct);
    }
    else
    {
      /* no remaining pbuf (chain)  */
      if(iec_struct->state == IEC_CLOSING)
      {
        /*  close tcp connection */
        iecsock_server_connection_close(tpcb, iec_struct);
      }
    }
    ret_err = ERR_OK;
  }
  else
  {
    /* nothing to be done */
    tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

/**
  * @brief  This function implements the tcp_sent LwIP callback (called when ACK
  *         is received from remote host for sent data) 
  * @param  None
  * @retval None
  */
static err_t iecsock_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  struct iecsock_struct *iec_struct;

  LWIP_UNUSED_ARG(len);

  iec_struct = (struct iecsock_struct *)arg;
  
  if(iec_struct->p != NULL)
  {
    /* still got pbufs to send */
    iecsock_server_send(tpcb, iec_struct);
  }
  else
  {
    /* if no more data to send and client closed connection*/
    if(iec_struct->state == IEC_CLOSING)
      iecsock_server_connection_close(tpcb, iec_struct);
  }
  return ERR_OK;
}


/**
  * @brief  This function is used to send data for tcp connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
static void iecsock_server_send(struct tcp_pcb *tpcb, struct iecsock_struct *iec_struct)
{
  struct pbuf *ptr;
  err_t wr_err = ERR_OK;
 
  while ((wr_err == ERR_OK) &&
         (iec_struct->p != NULL) && 
         (iec_struct->p->len <= tcp_sndbuf(tpcb)))
  {
    
    /* get pointer on pbuf from es structure */
    ptr = iec_struct->p;

    /* enqueue data for transmission */
    wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
    
    if (wr_err == ERR_OK)
    {
      u16_t plen;

      plen = ptr->len;
     
      /* continue with next pbuf in chain (if any) */
      iec_struct->p = ptr->next;
			
// 			Uncommenting this makes hard fault      
//      if(iec_struct->p != NULL)
//      {
//        /* increment reference count for es->p */
//        pbuf_ref(iec_struct->p);
//      }
      
      /* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
      pbuf_free(ptr);

      /* Update tcp window size to be advertized : should be called when received
      data (with the amount plen) has been processed by the application layer */
      tcp_recved(tpcb, plen);
   }
   else if(wr_err == ERR_MEM)
   {
      /* we are low on memory, try later / harder, defer to poll */
     iec_struct->p = ptr;
   }
   else
   {
     /* other problem ?? */
   }
  }
}

/**
  * @brief  This functions closes the tcp connection
  * @param  tcp_pcb: pointer on the tcp connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
static void iecsock_server_connection_close(struct tcp_pcb *tpcb, struct iecsock_struct *iec_struct)
{
  
  /* remove all callbacks */
  tcp_arg(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL, 0);
	NS = 0x00;
	NR = 0x00;
  
  /* delete es structure */
  if (iec_struct != NULL)
  {
    vPortFree(iec_struct);
  }  
  
  /* close tcp connection */
  tcp_close(tpcb);
}

#endif /* LWIP_TCP */
