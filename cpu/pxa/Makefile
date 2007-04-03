#
# (C) Copyright 2000, 2002
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

CPU_TYPE=xscale

ifeq ($(SOFTWARE_TYPE),WINCE)
STACKS_VALID = -DCONFIG_STACKS_VALID
else

ifeq ($(SOFTWARE_TYPE),GAME)
STACKS_VALID = -DCONFIG_STACKS_VALID
else
STACKS_VALID =
endif
endif
ARM_ELF_GCC     ?= $(CROSS_COMPILE)gcc

LIB	= lib$(CPU).a

ifeq ($(INCLUDE_MINIDEBUG),y)
START	= minidebug.o ministart.o
else
START	= start.o ministart.o
endif
OBJS	= serial.o interrupts.o cpu.o i2c.o mmc.o

ifdef PXALCD
   OBJS	+= pxafb.o
endif

ifeq ($(PLATFORM_TYPE),HALOGEN)
   OBJS	+= usb_ohci.o
else
ifeq ($(PLATFORM_TYPE),ARGON)
   OBJS	+= usb_ohci.o
else
ifeq ($(PLATFORM_TYPE),NEON270)
   OBJS	+= usb_ohci.o
else
ifeq ($(PLATFORM_TYPE),NEON)
   OBJS	+= usb_ohci.o
else
ifeq ($(PLATFORM_TYPE),NEONB)
   OBJS	+= usb_ohci.o
else
endif
endif
endif
endif
endif

SOBJS =  xmodemReceive.o

all:	.depend $(START) $(LIB)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $^

#########################################################################

.depend: Makefile $(START.o=.S) $(OBJS:.o=.c)
	$(CC) -M $(CFLAGS) $(START:.o=.S) $(OBJS:.o=.c) > $@

sinclude .depend

minidebug.o : minidebug.S Makefile
	$(ARM_ELF_GCC) -c $(D_GNUC) -DUBOOT=1 -DDISPLAY_TYPE=$(DISPLAY_TYPE) -DPLATFORM_TYPE=$(PLATFORM_TYPE) -DPLATFORM_REV=$(PLATFORM_REV) -DSOFTWARE_TYPE=$(SOFTWARE_TYPE) $(STACKS_VALID) -DCPU_CLOCK=$(CPU_CLOCK) \
		$(SOFT_FLOAT) -I$(TOPDIR)/include -mtune=$(CPU_TYPE) -mcpu=$(CPU_TYPE) -o $@ $<

#########################################################################
