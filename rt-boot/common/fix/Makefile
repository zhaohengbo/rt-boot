include $(TOPDIR)/config.mk

LIB	= libfix.a

START	=
OBJS	=
SOBJS	=

OBJS	+= fix_gcc.o
OBJS	+= fix_rtt.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
