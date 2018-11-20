include $(TOPDIR)/config.mk

LIB	= libuip-httpd.a

START	=
OBJS	=
SOBJS	=

OBJS	+= fs.o
OBJS	+= httpd.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
