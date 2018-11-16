include $(TOPDIR)/config.mk

LIB	= libcommon.a

START	=
OBJS	=
SOBJS	=

OBJS	+= init/init.o
OBJS	+= global/global.o
OBJS	+= main/main.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
