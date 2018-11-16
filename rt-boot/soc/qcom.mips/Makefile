#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= libsoc.a

OBJS	= common/global/global.o
OBJS   += common/timer/timer.o
OBJS   += common/net/ag71xx/ag71xx_phy.o
OBJS   += common/net/ag71xx/ag71xx_gmac.o
OBJS   += common/net/ag71xx/ag71xx_mdio.o
OBJS   += common/net/ag71xx/ag71xx_eth.o
OBJS   += common/net/ag71xx/ag71xx_debug.o
OBJS   += qca953x/clock/clock.o
OBJS   += qca953x/reset/reset.o
OBJS   += qca953x/ls_uart/ls_uart.o
OBJS   += qca953x/gpio/gpio.o
OBJS   += qca953x/init/init.o
OBJS   += qca953x/irq/irq.o

SOBJS	= 

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $(OBJS) $(SOBJS)

#########################################################################

.depend:	Makefile $(START:.o=.S) $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
