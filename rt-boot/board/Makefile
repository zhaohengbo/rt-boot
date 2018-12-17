include $(TOPDIR)/config.mk

LIB	= libboard.a

START	=
OBJS	=
SOBJS	=

OBJS	+= tl-wr810n_v2/init/init.o
OBJS	+= tl-wr810n_v2/console/console.o
OBJS	+= tl-wr810n_v2/network/network.o
OBJS	+= tl-wr810n_v2/spi/spi.o
OBJS	+= tl-wr810n_v2/flash/flash.o
OBJS	+= tl-wr810n_v2/loader/loader.o
OBJS	+= tl-wr810n_v2/gpio/gpio.o
OBJS	+= tl-wr810n_v2/reboot/reboot.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
