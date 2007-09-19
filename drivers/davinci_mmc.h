#ifndef __DAVINCI_MMC_H__
#define __DAVINCI_MMC_H__ "$Id$"

/*
 * davinci_mmc.h
 *
 * This header file declares ...
 *
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

// declared in include/mmc.h
int mmc_init(int verbose);
int mmc_read(ulong src, uchar *dst, int size);
int mmc_write(uchar *src, ulong dst, int size);
int mmc2info(ulong addr);


// declared in include/part.h
// block_dev_desc_t * mmc_get_dev(int dev); 

uchar *mmc_cmd(ushort cmd, uint arg, ushort cmdat);



#endif

