#ifndef __DHCPD_H__
#define __DHCPD_H__

#ifndef DHCPD_ADDR_START
#define  DHCPD_ADDR_START 100
#endif

#ifndef DHCPD_ADDR_END 
#define  DHCPD_ADDR_END 200
#endif

#define IP_BITMAP_SIZE ((DHCPD_ADDR_END-DHCPD_ADDR_START)/sizeof(u8_t)+1)
#define DHCPD_CHADDR_LEN 16U

struct dhcp_client{
 	u8_t hwaddr[DHCPD_CHADDR_LEN];
 	uip_ipaddr_t ipaddr;
    struct dhcp_client *next;
};

struct dhcpd_pack {
	u8_t serverid[4];
	u16_t lease_time[2];
	u16_t ipaddr[4];
	u16_t netmask[4];
	u16_t dnsaddr[4];
	u16_t default_router[4];
};

void dhcpd_init(void);
		
#endif

