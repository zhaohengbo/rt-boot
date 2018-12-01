include $(TOPDIR)/config.mk

LIB	= libdfs.a

START	=
OBJS	=
SOBJS	=

OBJS	+= dfs.o
OBJS	+= dfs_file.o
OBJS	+= dfs_fs.o
OBJS	+= dfs_posix.o

OBJS	+= filesystems/devfs/devfs.o
OBJS	+= filesystems/ramfs/dfs_ramfs.o
OBJS	+= filesystems/romfs/dfs_romfs.o
OBJS	+= filesystems/romfs/romfs.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
