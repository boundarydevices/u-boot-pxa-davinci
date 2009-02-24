
struct edid_detailed_timings
{
	unsigned char pixel_clock_lsb;
	unsigned char pixel_clock_msb;

	unsigned char xres_lsb;
	/* hblanking = left_margin + right_margin + hsync_width */
	unsigned char hblanking_lsb;
	/* high nibble xres, low nibble blanking */
	unsigned char xres_hblanking_msn;

	unsigned char yres_lsb;
	/* hblanking = upper_margin + lower_margin + vsync_width */
	unsigned char vblanking_lsb;
	/* high nibble yres, low nibble blanking */
	unsigned char yres_vblanking_msn;

	unsigned char rightmargin_lsb;
	unsigned char hsync_width_lsb;

	unsigned char lowermargin_vsync_lsn;
	unsigned char rightmargin_hsync_lowermargin_vsync_ms2;

	unsigned char hsize_lsb;
	unsigned char vsize_lsb;
	unsigned char hsize_vsize_msn;	/* high nibble hsize, low nibble vsize */

	unsigned char hborder;
	unsigned char vborder;
	unsigned char flags;
#define EDT_FLAGS_INTERLACED		0x80
#define EDT_FLAGS_SYNC_TYPE_MASK	0x18
#define EDT_FLAGS_HSYNC_POLARITY	0x04
#define EDT_FLAGS_VSYNC_POLARITY	0x02

};

unsigned static inline edt_pixel_clock(struct edid_detailed_timings *p)
{
	return ((p->pixel_clock_msb << 8) | p->pixel_clock_lsb) * 10000;
}

unsigned static inline edt_xres(struct edid_detailed_timings *p)
{
	return ((p->xres_hblanking_msn & 0xf0) << 4) | p->xres_lsb;
}
unsigned static inline edt_hblanking(struct edid_detailed_timings *p)
{
	return ((p->xres_hblanking_msn & 0x0f) << 8) | p->hblanking_lsb;
}
unsigned static inline edt_yres(struct edid_detailed_timings *p)
{
	return ((p->yres_vblanking_msn & 0xf0) << 4) | p->yres_lsb;
}
unsigned static inline edt_vblanking(struct edid_detailed_timings *p)
{
	return ((p->yres_vblanking_msn & 0x0f) << 8) | p->vblanking_lsb;
}
unsigned static inline edt_rightmargin(struct edid_detailed_timings *p)
{
	/* 10 bit field */
	return ((p->rightmargin_hsync_lowermargin_vsync_ms2 & 0xc0) << 2) |
		p->rightmargin_lsb;
}
unsigned static inline edt_hsync_width(struct edid_detailed_timings *p)
{
	/* 10 bit field */
	return ((p->rightmargin_hsync_lowermargin_vsync_ms2 & 0x30) << 4) |
		p->hsync_width_lsb;
}

unsigned static inline edt_lowermargin(struct edid_detailed_timings *p)
{
	/* 6 bit field */
	return ((p->rightmargin_hsync_lowermargin_vsync_ms2 & 0x0c) << 2) |
		(p->lowermargin_vsync_lsn >> 4);
}
unsigned static inline edt_vsync_width(struct edid_detailed_timings *p)
{
	/* 6 bit field */
	return ((p->rightmargin_hsync_lowermargin_vsync_ms2 & 0x03) << 4) |
		(p->lowermargin_vsync_lsn & 0x0f);
}
/*
 * hblanking = leftmargin + rightmargin + hsync_width
 * left_margin = hblanking - (rightmargin + hsync_width)
 */
unsigned static inline edt_leftmargin(struct edid_detailed_timings *p)
{
	return edt_hblanking(p) - (edt_rightmargin(p) + edt_hsync_width(p));
}
/*
 * vblanking = upper_margin + lowermargin + vsync_width
 * upper_margin = vblanking - (lower_margin + vsync_width)
 */
unsigned static inline edt_uppermargin(struct edid_detailed_timings *p)
{
	return edt_vblanking(p) - (edt_lowermargin(p) + edt_vsync_width(p));
}


unsigned static inline edt_hsize(struct edid_detailed_timings *p)
{
	return ((p->hsize_vsize_msn & 0xf0) << 4) | p->hsize_lsb;
}
unsigned static inline edt_vsize(struct edid_detailed_timings *p)
{
	return ((p->hsize_vsize_msn & 0x0f) << 8) | p->vsize_lsb;
}

unsigned static inline edt_interlaced(struct edid_detailed_timings *p)
{
	return (p->flags & EDT_FLAGS_INTERLACED);
}
unsigned static inline edt_sync_type(struct edid_detailed_timings *p)
{
	return (p->flags & EDT_FLAGS_SYNC_TYPE_MASK);
}
unsigned static inline edt_hsync_positive(struct edid_detailed_timings *p)
{
	return (p->flags & EDT_FLAGS_HSYNC_POLARITY);
}
unsigned static inline edt_vsync_positive(struct edid_detailed_timings *p)
{
	return (p->flags & EDT_FLAGS_VSYNC_POLARITY);
}
