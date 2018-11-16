include $(TOPDIR)/config.mk

LIB	= libfinsh.a

START	=
OBJS	=
SOBJS	=

OBJS	+= cmd.o
OBJS	+= msh.o
OBJS	+= msh_cmd.o
OBJS	+= msh_file.o
OBJS	+= shell.o
OBJS	+= symbol.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
