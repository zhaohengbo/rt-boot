include $(TOPDIR)/config.mk

LIB	= libdrivers.a

START	=
OBJS	=
SOBJS	=

OBJS	+= ipc/completion.o
OBJS	+= ipc/dataqueue.o
OBJS	+= ipc/pipe.o
OBJS	+= ipc/ringblk_buf.o
OBJS	+= ipc/ringbuffer.o
OBJS	+= ipc/waitqueue.o
OBJS	+= ipc/workqueue.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
