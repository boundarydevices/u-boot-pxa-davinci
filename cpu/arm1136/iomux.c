/*
 * Copyright 2004-2006 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <asm/hardware.h>
#include <asm/io.h>
#include "mx31_pins.h"
#include "iomux.h"

/*!
 * 4 control fields per MUX register
 */
#define MUX_CTL_FIELDS          4

/*!
 * 3 control fields per PAD register
 */
#define PAD_CTL_FIELDS          3

/*!
 * Maximum number of MUX pins
 * Number of pins = (highest iomux reg - lowest iomux reg + 1) * (4 pins/reg)
 */
#define MUX_PIN_NUM_MAX \
        (((u32 *)IOMUXSW_MUX_END - (u32 *)IOMUXSW_MUX_CTL + 1) * MUX_CTL_FIELDS)

/*!
 * Number of pad controls =
 *               (highest pad ctl reg - lowest pad ctl reg + 1) * (3 pins/reg)
 */
#define PAD_CTL_NUM_MAX \
        (((u32 *)IOMUXSW_PAD_END - (u32 *)IOMUXSW_PAD_CTL + 1) * PAD_CTL_FIELDS)

#define PIN_TO_IOMUX_INDEX(pin) ((pin >> MUX_I) & ((1 << (MUX_F - MUX_I)) - 1))
#define PIN_TO_IOMUX_FIELD(pin) ((pin >> MUX_F) & ((1 << (PAD_I - MUX_F)) - 1))

/*!
 * 8 bits for each MUX control field
 */
#define MUX_CTL_BIT_LEN         8

/*!
 * 10 bits for each PAD control field
 */
#define MUX_PAD_BIT_LEN         10

/*!
 * IOMUX register (base) addresses
 */
enum iomux_reg_addr {
	IOMUXGPR = IO_ADDRESS(IOMUXC_BASE_ADDR) + 0x008,	/*!< General purpose */
	IOMUXSW_MUX_CTL = IO_ADDRESS(IOMUXC_BASE_ADDR) + 0x00C,	/*!< MUX control */
	IOMUXSW_MUX_END = IO_ADDRESS(IOMUXC_BASE_ADDR) + 0x150,	/*!< last MUX control register */
	IOMUXSW_PAD_CTL = IO_ADDRESS(IOMUXC_BASE_ADDR) + 0x154,	/*!< Pad control */
	IOMUXSW_PAD_END = IO_ADDRESS(IOMUXC_BASE_ADDR) + 0x308,	/*!< last Pad control register */
	IOMUXINT_OBS1 = IO_ADDRESS(IOMUXC_BASE_ADDR) + 0x000,	/*!< Observe interrupts 1 */
	IOMUXINT_OBS2 = IO_ADDRESS(IOMUXC_BASE_ADDR) + 0x004,	/*!< Observe interrupts 2 */
};

/* len - mask bit length; fld - mask bit field. Example, to have the mask:
 * 0xFF000000, use GET_FIELD_MASK(8, 3). Translate in plain language:
 * "set the 3rd (0-based) 8-bit-long field to all 1's */
#define GET_FIELD_MASK(len, fld)    (((1 << len) - 1) << (len * fld))

/*!
 * This function is used to configure a pin through the IOMUX module.
 * FIXED ME: for backward compatible. Will be static function!
 * @param  pin		a pin number as defined in \b #iomux_pin_name_t
 * @param  out		an output function as defined in \b #iomux_pin_ocfg_t
 * @param  in		an input function as defined in \b #iomux_pin_icfg_t
 *
 * @return 		0 if successful; Non-zero otherwise
 */
int iomux_config_mux(iomux_pin_name_t pin, iomux_pin_ocfg_t out,
		     iomux_pin_icfg_t in)
{
	u32 reg, l, ret = 0;
	u32 mux_index = PIN_TO_IOMUX_INDEX(pin);
	u32 mux_field = PIN_TO_IOMUX_FIELD(pin);
	u32 mux_mask = GET_FIELD_MASK(MUX_CTL_BIT_LEN, mux_field);

//	MXC_ERR_CHK((mux_index > (MUX_PIN_NUM_MAX / MUX_CTL_FIELDS - 1)) ||
//		    (mux_field >= MUX_CTL_FIELDS));

	reg = IOMUXSW_MUX_CTL + (mux_index * 4);
//	spin_lock(&gpio_mux_lock);
	l = __raw_readl(reg);
	l = (l & (~mux_mask)) |
	    (((out << 4) | in) << (mux_field * MUX_CTL_BIT_LEN));
	__raw_writel(l, reg);
//	spin_unlock(&gpio_mux_lock);

	return ret;
}

/*!
 * Request ownership for an IO pin. This function has to be the first one
 * being called before that pin is used. The caller has to check the
 * return value to make sure it returns 0.
 *
 * @param  pin		a name defined by \b iomux_pin_name_t
 * @param  out		an output function as defined in \b #iomux_pin_ocfg_t
 * @param  in		an input function as defined in \b #iomux_pin_icfg_t
 *
 * @return		0 if successful; Non-zero otherwise
 */
int mxc_request_iomux(iomux_pin_name_t pin, iomux_pin_ocfg_t out,
		      iomux_pin_icfg_t in)
{
	int ret = iomux_config_mux(pin, out, in);
	return ret;
}

