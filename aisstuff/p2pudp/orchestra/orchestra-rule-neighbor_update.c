/*
 *
 */

#include "contiki.h"
#include "orchestra.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/packetbuf.h"
#include "sys/log.h"
#define LOG_MODULE "Orchestra"
#define LOG_LEVEL  LOG_LEVEL_INFO



/*---------------------------------------------------------------------------*/
static void
neighbor_updated(const linkaddr_t *linkaddr, uint8_t is_added)
{
  //uip_ipaddr_t *ptr = NULL;
  if(is_added) {
    LOG_INFO("neighbor_updated, added: ");
  } else {
    LOG_INFO("neighbor_updated, removed: ");
  }
  LOG_INFO_LLADDR(linkaddr);
  LOG_INFO("\n");
  /*LOG_INFO("Looking up IP\n");
  ptr = uip_ds6_nbr_ipaddr_from_lladdr((const uip_lladdr_t *)linkaddr);
  if (ptr != NULL)
  {
    LOG_INFO_6ADDR(ptr);
    LOG_INFO("\n");
    LOG_INFO("IP address\n");
  }
  else
  {
    LOG_INFO("IP address not found\n");
  }*/
}
/*---------------------------------------------------------------------------*/
static void
child_added(const linkaddr_t *linkaddr)
{
  LOG_INFO("Child added: ");
  LOG_INFO_LLADDR(linkaddr);
  LOG_INFO("\n");
}
/*---------------------------------------------------------------------------*/
static void
child_removed(const linkaddr_t *linkaddr)
{
    LOG_INFO("Child removed: ");
    LOG_INFO_LLADDR(linkaddr);
    LOG_INFO("\n");
}
/*---------------------------------------------------------------------------*/

static void
new_time_source(const struct tsch_neighbor *old, const struct tsch_neighbor *new)
{
    // Assume Time Source is RPL Parent
    if(new != old) {
      const linkaddr_t *old_addr = tsch_queue_get_nbr_address(old);
      const linkaddr_t *new_addr = tsch_queue_get_nbr_address(new);
      if(new_addr != NULL) {
        // Remove old parent
        // Add new parent

      } else {
        // Remove old parent
        // No new parent found
      }
     LOG_INFO("Old parent: ");
     LOG_INFO_LLADDR(old_addr);
     LOG_INFO("\n");
     LOG_INFO("New parent: ");
     LOG_INFO_LLADDR(new_addr);
     LOG_INFO("\n");

    }
}


/*---------------------------------------------------------------------------*/
static void
init(uint16_t sf_handle)
{
  LOG_INFO("init: neighbor_updated\n");
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
