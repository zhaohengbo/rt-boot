/*
 * Ori Version mleaf90@gmail.com
 * All rights reserved.
 */

#ifndef __DHCPD_H__
#define __DHCPD_H__

typedef unsigned int u32_t;

#ifndef DHCPD_ADDR_START
#define  DHCPD_ADDR_START 100
#endif

#ifndef DHCPD_ADDR_END 
#define  DHCPD_ADDR_END 200
#endif

#define IP_BITMAP_SIZE ((DHCPD_ADDR_END-DHCPD_ADDR_START)/sizeof(u8_t)+1)
#define DHCPD_CHADDR_LEN 16U

struct ip_addr {
  u32_t addr;
};

struct dhcp_client{
 	u8_t hwaddr[DHCPD_CHADDR_LEN];
 	u8_t ip_bitmap[IP_BITMAP_SIZE];
 	struct ip_addr ipaddr;
    struct dhcp_client *next;
};

struct dhcpd_state {
  char state;
  struct uip_udp_conn *conn;
  u16_t ticks;
  u8_t mac_addr[16];
  int mac_len;
  u32_t xid;
  u8_t serverid[4];
  u16_t lease_time[2];
  u16_t ipaddr[4];
  u16_t netmask[4];
  u16_t dnsaddr[4];
  u16_t default_router[4];
};

void dhcpd_init(void);
		
#endif

