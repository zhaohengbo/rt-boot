include $(TOPDIR)/config.mk

LIB	= libuip.a

START	=
OBJS	=
SOBJS	=

OBJS	+= uip.o
OBJS	+= uip_arch.o
OBJS	+= uip_arp.o
OBJS	+= uip_socket.o
OBJS	+= uip_main.o
OBJS	+= uip-neighbor.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
