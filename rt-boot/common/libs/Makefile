include $(TOPDIR)/config.mk

LIB	= liblibs.a

START	=
OBJS	=
SOBJS	=

OBJS	+= sfud/sfud.o
OBJS	+= sfud/sfud_sfdp.o

OBJS	+= tinycrypt/tiny_aes.o
OBJS	+= tinycrypt/tiny_base64.o
OBJS	+= tinycrypt/tiny_md5.o
OBJS	+= tinycrypt/tiny_sha1.o
OBJS	+= tinycrypt/tiny_sha2.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
