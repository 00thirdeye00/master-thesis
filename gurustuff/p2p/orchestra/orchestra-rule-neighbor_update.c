/*
 *
 */

#include "contiki.h"
#include "orchestra.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip.h"
#include "net/packetbuf.h"
#include "sys/log.h"
#define LOG_MODULE "Orchestra"
#define LOG_LEVEL LOG_LEVEL_INFO


extern void nbr_construction(const uip_ipaddr_t *ipaddr);


/*---------------------------------------------------------------------------*/

static void
neighbor_updated(const linkaddr_t *linkaddr, uint8_t is_added)
{
  const  uip_ds6_addr_t *llprefix = uip_ds6_get_link_local(-1); 
  static uip_ipaddr_t ipaddr; 
  // LOG_INFO_6ADDR(&ptr->ipaddr); // Log the link local ip-address   
  // LOG_INFO("\n");    

  LOG_DBG("Enter: Neighbor updated\n");

  uip_ip6addr_copy(&ipaddr, &llprefix->ipaddr);
  uip_ds6_set_addr_iid(&ipaddr, (const uip_lladdr_t*)linkaddr);

  LOG_DBG("Neighbor updated: "); // Log the link local ip-address   
  LOG_DBG_6ADDR(&ipaddr); // Log the link local ip-address   
  LOG_DBG("\n");      

  LOG_INFO("is_added: %d\n", is_added);
  LOG_INFO_6ADDR(&ipaddr); // Log the link local ip-address   
  LOG_INFO("\n");  

 if (is_added) {
    LOG_DBG("neighbor_updated, added: \n");

    nbr_construction(&ipaddr);
    /******************
     * Add your own a call to you function for adding a new neighbor node
     * 
     * new_neighbor(ipaddr)
    ******************/
  } else {
    LOG_ERR("neighbor_updated, removed address: ");
    LOG_ERR_6ADDR(&ipaddr); // Log the link local ip-address   
    LOG_ERR("\n");
  }

//   LOG_INFO_6ADDR(ipaddr); // Log the link local ip-address   
}
/*---------------------------------------------------------------------------*/
static void
child_added(const linkaddr_t *linkaddr)
{
  // Not used in storing mode
  LOG_INFO("Child added: ");
  LOG_INFO_LLADDR(linkaddr);
  LOG_INFO("\n");
}
/*---------------------------------------------------------------------------*/
static void
child_removed(const linkaddr_t *linkaddr)
{
  // Not used in storing mode
  LOG_INFO("Child removed: ");
  LOG_INFO_LLADDR(linkaddr);
  LOG_INFO("\n");
}
/*---------------------------------------------------------------------------*/

static void
new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new)
{
  if (new != old) {
    // const linkaddr_t *old_addr = tsch_queue_get_nbr_address(old);
    const linkaddr_t *new_addr = tsch_queue_get_nbr_address(new);
    if (new_addr != NULL)  {      
      // There is a new parent
      // LOG_INFO_LLADDR(new_addr);      
    }
    else {
      // No new parent found
    }
    // LOG_INFO("\n");
  }
}

/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
  LOG_INFO("init: update_neighbor\n");
}
/*---------------------------------------------------------------------------*/
struct orchestra_rule update_neighbor = {
    init,
    new_time_source,
    NULL,
    child_added,
    child_removed,
    neighbor_updated,
    NULL,
    "unicast per neighbor update",
    0,
};
