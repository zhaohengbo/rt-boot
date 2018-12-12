include $(TOPDIR)/config.mk

LIB	= libwebnet.a

START	=
OBJS	=
SOBJS	=

OBJS	+= webnet.o
OBJS	+= wn_mimetype.o
OBJS	+= wn_module.o
OBJS	+= wn_request.o
OBJS	+= wn_session.o
OBJS	+= wn_utils.o
OBJS	+= wn_main.o

OBJS	+= module/wn_module_alias.o
OBJS	+= module/wn_module_asp.o
OBJS	+= module/wn_module_auth.o
OBJS	+= module/wn_module_cgi.o
OBJS	+= module/wn_module_dav.o
OBJS	+= module/wn_module_index.o
OBJS	+= module/wn_module_log.o
OBJS	+= module/wn_module_lua.o
OBJS	+= module/wn_module_ssi.o
OBJS	+= module/wn_module_upload.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
