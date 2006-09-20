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
OBJS	= interrupts.o cpu.o mmc.o iomux.o 

SOBJS =  xmodemReceive.o

all:	.depend $(START) $(LIB) $(SOBJS)

$(LIB):	$(OBJS) $(SOBJS)
	$(AR) crv $@ $^

.depend: Makefile $(START:.o=.S) $(OBJS:.o=.c) $(SOBJS:.o=.S)
	$(CC) -M $(CFLAGS) $(filter-out Makefile,$^) > $@

sinclude .depend

minidebug.o : minidebug.S Makefile
	echo $(START)
	$(ARM_ELF_GCC) -c $(D_GNUC) -DDISPLAY_TYPE=$(DISPLAY_TYPE) -DPLATFORM_TYPE=$(PLATFORM_TYPE) -DPLATFORM_REV=$(PLATFORM_REV) -DSOFTWARE_TYPE=$(SOFTWARE_TYPE) $(STACKS_VALID) -DCPU_CLOCK=$(CPU_CLOCK) \
		$(SOFT_FLOAT) -I$(TOPDIR)/include -mtune=$(CPU_TYPE) -mcpu=$(CPU_TYPE) -o $@ $<

