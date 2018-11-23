#include <kernel/rtthread.h>

#include <net/uip/uip.h>
#include <net/uip-dhcpd/dhcpd.h>

struct dhcp_client *head_dhcpd;
typedef struct dhcp_client dhcp_client_node;

static struct dhcpd_pack s;
static u8_t ip_bitmap[IP_BITMAP_SIZE];

struct dhcpd_msg {
  u8_t op, htype, hlen, hops;
  u8_t xid[4];
  u16_t secs;
  u16_t flags;
  u8_t ciaddr[4];
  u8_t yiaddr[4];
  u8_t siaddr[4];
  u8_t giaddr[4];
  u8_t chaddr[16];
  u8_t sname[64];
  u8_t file[128];
  u8_t options[312];
};
/*
OP:
1 DHCP DISCOVER
2 DHCP OFFER
3 DHCP REQUEST
4 DHCPDECLINE
5 DHCPACK
6 DHCPNACK
7 DHCPRELEASE
*/
#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPC_SERVER_PORT  67
#define DHCPC_CLIENT_PORT  68

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7

#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_DOMAIN_NAME  15
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_LEASE_TIME   51
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_END         255
#define DHCP_OPTION_OVER		0x34

static  uip_ipaddr_t uip_server_ipaddr;
static  uip_ipaddr_t uip_dhcpd_ipaddr;
static  uip_ipaddr_t uip_server_netmask;
static  u16_t uip_dhcp_leasetime = 3600;	//1h

//#define DHCPD_DEBUG

#ifdef DHCPD_DEBUG
#define uip_dhcpd_printf(fmt,args...) rt_kprintf(fmt,##args)
#else
#define uip_dhcpd_printf(fmt,args...)
#endif

static u8_t *add_msg_type(u8_t *optptr, u8_t type)
{
	*optptr++ = DHCP_OPTION_MSG_TYPE;
	*optptr++ = 1;
	*optptr++ = type;
	return optptr;
}

static u8_t *add_dhcpd_server_id(u8_t *optptr)
{
	*optptr++ = DHCP_OPTION_SERVER_ID;
	*optptr++ = 4;
	uip_ipaddr_copy(optptr, uip_server_ipaddr);
	return optptr + 4;
}

static u8_t *add_dhcpd_default_router(u8_t *optptr)
{
	*optptr++ = DHCP_OPTION_ROUTER;
	*optptr++ = 4;
	uip_ipaddr_copy(optptr, uip_server_ipaddr);
	return optptr + 4;
}

static u8_t *add_dhcpd_dns_server(u8_t *optptr)
{
	*optptr++ = DHCP_OPTION_DNS_SERVER;
	*optptr++ = 4;
	uip_ipaddr_copy(optptr, uip_server_ipaddr);
	return optptr + 4;
}

static u8_t *add_dhcpd_domain_name(u8_t *optptr)
{
  *optptr++ = DHCP_OPTION_DOMAIN_NAME;
  *optptr++ = 7;
  rt_memcpy(optptr, "rt-boot", 7);
  return optptr + 7;
}

static u8_t *add_dhcpd_lease_time(u8_t *optptr)
{
	u16_t leasetime;
	*optptr++ = DHCP_OPTION_LEASE_TIME;
	*optptr++ = 4;
	*optptr++ = 0;
	*optptr++ = 0;
	leasetime=HTONS(uip_dhcp_leasetime);
	optptr = rt_memcpy(optptr, &leasetime, 2);
	return optptr + 2;
}

static u8_t *add_dhcpd_subnet_mask(u8_t *optptr)
{
	*optptr++ = DHCP_OPTION_SUBNET_MASK;
	*optptr++ = 4;
	uip_ipaddr_copy(optptr, uip_server_netmask);
	return optptr + 4;
}

static u8_t *add_end(u8_t *optptr)
{
	*optptr++ = DHCP_OPTION_END;
	return optptr;
}

void display_list(dhcp_client_node *head) {
    dhcp_client_node *p;
    for (p=head->next; p!=RT_NULL; p=p->next)
	{
		uip_dhcpd_printf("ipaddr: %d.%d.%d.%d\n",
	  		uip_ipaddr1(p->ipaddr),uip_ipaddr2(p->ipaddr),
			uip_ipaddr3(p->ipaddr),uip_ipaddr4(p->ipaddr));
	}
    uip_dhcpd_printf("\n");
}

int find_vaild_hwaddr(dhcp_client_node* head,char *HwAddress)
{
	dhcp_client_node *p;
	p =head;
	while((p->next)&&(rt_strcmp((char *)p->next->hwaddr,HwAddress)))
	{
		p = p->next;
	}
	if(p->next)
	{
		uip_dhcpd_printf("Found match ipaddr: %d.%d.%d.%d\n",
			uip_ipaddr1(p->ipaddr),uip_ipaddr2(p->ipaddr),
			uip_ipaddr3(p->ipaddr),uip_ipaddr4(p->ipaddr));
		return 0;
	}
	else
	{
		uip_dhcpd_printf("Could not find vaild ipaddr\n");
		return -1;
	}
	
}

int list_add(dhcp_client_node **rootp, uip_ipaddr_t ipaddr, u8_t *hw_addr)  
{  
    dhcp_client_node *newdhcp_client_node;
    dhcp_client_node *previous;
    dhcp_client_node *current;
  
    current = *rootp;  
    previous = RT_NULL;  
  
    while ((current != RT_NULL) && (uip_ipaddr4(current->ipaddr) < uip_ipaddr4(ipaddr)))  
    {  
        previous = current;  
        current = current->next;  
    }  
  
    newdhcp_client_node = (dhcp_client_node *)rt_calloc(sizeof(dhcp_client_node),1);  
    if (newdhcp_client_node == RT_NULL) { 
        return -1; 
	} 
	uip_ipaddr_copy(newdhcp_client_node->ipaddr, ipaddr);
	rt_memcpy(newdhcp_client_node->hwaddr,hw_addr,DHCPD_CHADDR_LEN);

    newdhcp_client_node->next = current;  
    if (previous == RT_NULL){
		
        *rootp = newdhcp_client_node;
    }
    else{
		
        previous->next = newdhcp_client_node;  
    }
  
    return 0;  
}

static int search_vaild_bitmap(u8_t *bitmap,int size)
{
	int i=0,index=0;
	for(index=0;index<size;index++,bitmap++){
		i=0;
		while(i<8){
			if((*bitmap)&(1<<i++))
				continue;
			*bitmap=(*bitmap)|(1<<(--i));
			return ((index*8)+i);
		}
	}
	return -1;
}

static int dhcpd_relay_offer(dhcp_client_node* head)
{
	u8_t *end;
    uip_ipaddr_t addr;
	dhcp_client_node *p;
	p =head;
	struct dhcpd_msg *m = (struct dhcpd_msg *)uip_appdata;
	
    while((p->next)&&(rt_strcmp((char *)p->next->hwaddr,(char *)m->chaddr)))
	{
     	p = p->next;
	}
	if(p->next)
	{
		uip_ipaddr_copy(addr, p->next->ipaddr);
		uip_dhcpd_printf("offer used ipaddr: %d.%d.%d.%d\n",
			uip_ipaddr1(addr),uip_ipaddr2(addr),
			uip_ipaddr3(addr),uip_ipaddr4(addr));
	}
	else
	{
		int ret;
		u8_t ip_addr4;
		ret=search_vaild_bitmap(ip_bitmap,IP_BITMAP_SIZE);
		if((ret<0)||((ret+DHCPD_ADDR_START)>DHCPD_ADDR_END)){
			uip_dhcpd_printf("allocate ip vaild bitmap failed.\n");
			return -1;
		}
		ip_addr4 = uip_ipaddr4(uip_dhcpd_ipaddr);
		ip_addr4 += (ret+DHCPD_ADDR_START);
		uip_ipaddr(addr, uip_ipaddr1(uip_dhcpd_ipaddr),
					uip_ipaddr2(uip_dhcpd_ipaddr),
					uip_ipaddr3(uip_dhcpd_ipaddr),
					ip_addr4);

		uip_dhcpd_printf("allocate new ip addr:%d.%d.%d.%d\n",
			uip_ipaddr1(addr),uip_ipaddr2(addr),
			uip_ipaddr3(addr),uip_ipaddr4(addr));
		list_add(&head,addr,m->chaddr);
	}

	m->op = DHCP_REPLY;
	rt_memcpy(m->yiaddr,&addr, sizeof(m->yiaddr));

	end = add_msg_type(&m->options[4], DHCPOFFER);
	end = add_dhcpd_server_id(end);
	end = add_dhcpd_default_router(end);
    end = add_dhcpd_dns_server(end);
	end = add_dhcpd_lease_time(end);
	end = add_dhcpd_subnet_mask(end);
	end = add_dhcpd_domain_name(end);
	end = add_end(end);
	
	uip_udp_broadcast(HTONS(DHCPC_CLIENT_PORT) ,uip_appdata, end - (u8_t *)uip_appdata);
	return 1;
	
}

static int dhcpd_replay_ask(dhcp_client_node* head)
{
	u8_t *end;
    uip_ipaddr_t addr;
	dhcp_client_node *p;
	p =head;

	struct dhcpd_msg *m = (struct dhcpd_msg *)uip_appdata;
	
	m->op = DHCP_REPLY;

 	while((p->next)&&(rt_strcmp((char *)p->next->hwaddr,(char *)m->chaddr)))
     	p = p->next;
	if(p->next)
	{
		uip_dhcpd_printf("Find available ipaddr from list\n");
		uip_ipaddr_copy(addr, p->next->ipaddr);
		uip_dhcpd_printf("uip dhcpd set client ipaddr: %d.%d.%d.%d\n",
			uip_ipaddr1(addr),uip_ipaddr2(addr),
			uip_ipaddr3(addr),uip_ipaddr4(addr));
		rt_memcpy(m->yiaddr, &addr, sizeof(m->yiaddr));
		end = add_msg_type(&m->options[4], DHCPACK);
		end = add_dhcpd_server_id(end);
		end = add_dhcpd_default_router(end);
		end = add_dhcpd_dns_server(end);
		end = add_dhcpd_lease_time(end);
		end = add_dhcpd_subnet_mask(end);
	}
	else
	{
		uip_dhcpd_printf("Could not find available ipaddr from list\n");
		uip_dhcpd_printf("send DHCPNAK\n");
		end = add_msg_type(&m->options[4], DHCPNAK);
	  	end = add_dhcpd_server_id(end);
	}

	end = add_end(end);
	
	uip_udp_broadcast(HTONS(DHCPC_CLIENT_PORT) ,uip_appdata, end - (u8_t *)uip_appdata);

	return 1;

}

static u8_t parse_dhcp_options(u8_t *optptr, int len)
{
  u8_t *end = optptr + len;
  u8_t type = 0;

  while(optptr < end) {
    switch(*optptr) {
    case DHCP_OPTION_SUBNET_MASK:
		rt_memcpy(s.netmask, optptr + 2, 4);
		break;
    case DHCP_OPTION_ROUTER:
		rt_memcpy(s.default_router, optptr + 2, 4);
		break;
    case DHCP_OPTION_DNS_SERVER:
		rt_memcpy(s.dnsaddr, optptr + 2, 4);
		break;
    case DHCP_OPTION_MSG_TYPE:
		type = *(optptr + 2);
		break;
    case DHCP_OPTION_SERVER_ID:
		rt_memcpy(s.serverid, optptr + 2, 4);
		break;
    case DHCP_OPTION_LEASE_TIME:
		rt_memcpy(s.lease_time, optptr + 2, 4);
		break;
	case DHCP_OPTION_REQ_IPADDR:
		rt_memcpy(s.ipaddr, optptr + 2, 4);
		break;
    case DHCP_OPTION_END:
      return type;
    }

    optptr += optptr[1] + 2;
  }
  return type;
}

static u8_t parse_dhcp_msg(void)
{
  struct dhcpd_msg *m = (struct dhcpd_msg *)uip_appdata;
  if(m->op == DHCP_REQUEST) {
    return parse_dhcp_options(&m->options[4], uip_datalen());
  }
  return 0;
}

void 
handle_dhcpd(void)
{
	unsigned char state;

	if(uip_newdata()){
		
		state=parse_dhcp_msg();

		if (state == DHCPDISCOVER){
			
			dhcpd_relay_offer(head_dhcpd);

		}
		else if(state == DHCPREQUEST){

			dhcpd_replay_ask(head_dhcpd);

		}
	}
	return;
}

void uip_dhcpd_appcall(void)
{
	if (uip_udp_conn->lport == HTONS(DHCPC_SERVER_PORT))
	{
		if(uip_udp_conn->rport == HTONS(DHCPC_CLIENT_PORT))
			handle_dhcpd();
	}
	return;
}

void dhcpd_init(void)
{
	head_dhcpd=(dhcp_client_node *)rt_calloc(sizeof(dhcp_client_node),1);
	if (head_dhcpd==RT_NULL) {
		rt_kprintf("allocated head_dhcpd failed.\n");
		return ;
	}
	head_dhcpd->next=RT_NULL;
	
	uip_ipaddr(uip_server_ipaddr, 192,168,1,1);
	uip_ipaddr(uip_dhcpd_ipaddr, 192,168,1,1);
	uip_ipaddr(uip_server_netmask, 255,255,255,0);

	if(uip_udp_new(HTONS(DHCPC_SERVER_PORT)) != RT_NULL) {
		register_udp_appcall(uip_dhcpd_appcall);
	}
}
