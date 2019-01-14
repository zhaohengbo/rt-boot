include $(TOPDIR)/config.mk

LIB	= liblwip.a

START	=
OBJS	=
SOBJS	=

OBJS	+= api/api_lib.o
OBJS	+= api/api_msg.o
OBJS	+= api/err.o
OBJS	+= api/if_api.o
OBJS	+= api/netbuf.o
OBJS	+= api/netdb.o
OBJS	+= api/netifapi.o
OBJS	+= api/sockets.o
OBJS	+= api/tcpip.o
OBJS	+= arch/sys_arch.o
OBJS	+= core/ipv4/acd.o
OBJS	+= core/ipv4/autoip.o
OBJS	+= core/ipv4/dhcp.o
OBJS	+= core/ipv4/etharp.o
OBJS	+= core/ipv4/icmp.o
OBJS	+= core/ipv4/igmp.o
OBJS	+= core/ipv4/ip4.o
OBJS	+= core/ipv4/ip4_addr.o
OBJS	+= core/ipv4/ip4_frag.o
OBJS	+= core/ipv6/dhcp6.o
OBJS	+= core/ipv6/ethip6.o
OBJS	+= core/ipv6/icmp6.o
OBJS	+= core/ipv6/inet6.o
OBJS	+= core/ipv6/ip6.o
OBJS	+= core/ipv6/ip6_addr.o
OBJS	+= core/ipv6/ip6_frag.o
OBJS	+= core/ipv6/mld6.o
OBJS	+= core/ipv6/nd6.o
OBJS	+= core/altcp.o
OBJS	+= core/altcp_alloc.o
OBJS	+= core/altcp_tcp.o
OBJS	+= core/def.o
OBJS	+= core/dns.o
OBJS	+= core/inet_chksum.o
OBJS	+= core/init.o
OBJS	+= core/ip.o
OBJS	+= core/mem.o
OBJS	+= core/memp.o
OBJS	+= core/netif.o
OBJS	+= core/pbuf.o
OBJS	+= core/raw.o
OBJS	+= core/stats.o
OBJS	+= core/sys.o
OBJS	+= core/tcp.o
OBJS	+= core/tcp_in.o
OBJS	+= core/tcp_out.o
OBJS	+= core/timeouts.o
OBJS	+= core/udp.o
OBJS	+= netif/ethernet.o
OBJS	+= netif/ethernetif.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
