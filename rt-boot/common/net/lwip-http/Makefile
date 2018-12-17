include $(TOPDIR)/config.mk

LIB	= liblwip-http.a

START	=
OBJS	=
SOBJS	=

OBJS	+= webclient.o
OBJS	+= webclient_file.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
