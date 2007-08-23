#
# (C) Copyright 2000-2006
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

LIB	= $(obj)lib$(CPU).a

START	= start.o
COBJS	= serial.o interrupts.o cpu.o i2c.o pxafb.o mmc.o usb.o

ifdef CONFIG_PXA27X
        COBJS += usb_ohci.o
else
ifdef CONFIG_SM501
#        COBJS += usb_ohci.o
endif
endif

ifeq ($(SOFTWARE_TYPE),WINCE)
STACKS_VALID = -DCONFIG_STACKS_VALID
else
ifeq ($(SOFTWARE_TYPE),GAME)
STACKS_VALID = -DCONFIG_STACKS_VALID
else
STACKS_VALID = -DCONFIG_STACKS_VALID
endif
endif

CPU_TYPE=xscale
ARM_ELF_GCC     ?= $(CROSS_COMPILE)gcc

ifeq ($(INCLUDE_MINIDEBUG),y)
START	= minidebug.o ministart.o
else
START	= start.o ministart.o
endif

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

minidebug.o: minidebug.S
	$(ARM_ELF_GCC) -c $(D_GNUC) -DUBOOT=1 -DDISPLAY_TYPE=$(DISPLAY_TYPE) -DPLATFORM_TYPE=$(PLATFORM_TYPE) -DPLATFORM_REV=$(PLATFORM_REV) -DSOFTWARE_TYPE=$(SOFTWARE_TYPE) $(STACKS_VALID) -DCPU_CLOCK=$(CPU_CLOCK) \
		$(SOFT_FLOAT) -I$(TOPDIR)/include -mtune=$(CPU_TYPE) -mcpu=$(CPU_TYPE) -o $@ $<

#########################################################################
