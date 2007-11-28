/*
 * (C) Copyright 2004
 * Jian Zhang, Texas Instruments, jzhang@ti.com.

 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>

#if defined(CFG_ENV_IS_IN_NAND) /* Environment is in Nand Flash */

#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <malloc.h>
#include <nand.h>

#if defined(CONFIG_CMD_ENV) && defined(CONFIG_CMD_NAND)
#define CMD_SAVEENV
#elif defined(CFG_ENV_OFFSET_REDUND)
#error Cannot use CFG_ENV_OFFSET_REDUND without CONFIG_CMD_ENV & CONFIG_CMD_NAND
#endif

#if defined(CFG_ENV_SIZE_REDUND) && (CFG_ENV_SIZE_REDUND != CFG_ENV_SIZE)
#error CFG_ENV_SIZE_REDUND should be the same as CFG_ENV_SIZE
#endif

#ifdef CONFIG_INFERNO
#error CONFIG_INFERNO not supported yet
#endif

int nand_legacy_rw (struct nand_chip* nand, int cmd,
	    size_t start, size_t len,
	    size_t * retlen, u_char * buf);

/* info for NAND chips, defined in drivers/nand/nand.c */
extern nand_info_t nand_info[];

/* references to names in env_common.c */
extern uchar default_environment[];
extern int default_environment_size;

char * env_name_spec = "NAND";


#ifdef ENV_IS_EMBEDDED
extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);
#else /* ! ENV_IS_EMBEDDED */
env_t *env_ptr = 0;
#endif /* ENV_IS_EMBEDDED */


/* local functions */
#if !defined(ENV_IS_EMBEDDED)
static void use_default(void);
#endif

DECLARE_GLOBAL_DATA_PTR;

uchar env_get_char_spec (int index)
{
	return ( *((uchar *)(gd->env_addr + index)) );
}


/* this is called before nand_init()
 * so we can't read Nand to validate env data.
 * Mark it OK for now. env_relocate() in env_common.c
 * will call our relocate function which will does
 * the real validation.
 *
 * When using a NAND boot image (like sequoia_nand), the environment
 * can be embedded or attached to the U-Boot image in NAND flash. This way
 * the SPL loads not only the U-Boot image from NAND but also the
 * environment.
 */
int env_init(void)
{
#if defined(ENV_IS_EMBEDDED)
	ulong total;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env1, *tmp_env2;

	total = CFG_ENV_SIZE;

	tmp_env1 = env_ptr;
	tmp_env2 = (env_t *)((ulong)env_ptr + CFG_ENV_SIZE);

	crc1_ok = (crc32(0, tmp_env1->data, ENV_SIZE) == tmp_env1->crc);
	crc2_ok = (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc);

	if (!crc1_ok && !crc2_ok)
		gd->env_valid = 0;
	else if(crc1_ok && !crc2_ok)
		gd->env_valid = 1;
	else if(!crc1_ok && crc2_ok)
		gd->env_valid = 2;
	else {
		/* both ok - check serial */
		if(tmp_env1->flags == 255 && tmp_env2->flags == 0)
			gd->env_valid = 2;
		else if(tmp_env2->flags == 255 && tmp_env1->flags == 0)
			gd->env_valid = 1;
		else if(tmp_env1->flags > tmp_env2->flags)
			gd->env_valid = 1;
		else if(tmp_env2->flags > tmp_env1->flags)
			gd->env_valid = 2;
		else /* flags are equal - almost impossible */
			gd->env_valid = 1;
	}

	if (gd->env_valid == 1)
		env_ptr = tmp_env1;
	else if (gd->env_valid == 2)
		env_ptr = tmp_env2;
#else /* ENV_IS_EMBEDDED */
	gd->env_addr  = (ulong)&default_environment[0];
	gd->env_valid = 1;
#endif /* ENV_IS_EMBEDDED */

	return (0);
}

#ifdef CMD_SAVEENV
/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
int saveenv(void)
{
	int ret = 0;
	int byteCnt;
#ifdef CFG_ENV_OFFSET_REDUND
	unsigned long offset = (gd->env_valid == 1)? CFG_ENV_OFFSET_REDUND : CFG_ENV_OFFSET;
#else
	const unsigned long offset = CFG_ENV_OFFSET;
#endif

#if defined(CFG_ENV_SECT_SIZE) && (CFG_ENV_SECT_SIZE > CFG_ENV_SIZE)
	ulong total = CFG_ENV_SECT_SIZE;
	ulong sectOffset = offset & (CFG_ENV_SECT_SIZE-1);
	uchar env_buffer[CFG_ENV_SECT_SIZE];
	offset &= ~(CFG_ENV_SECT_SIZE-1);		//aligned offset to sector boundary
#else
	ulong total = CFG_ENV_SIZE;
//	uchar *env_buffer = (uchar *)env_ptr;
#endif	/* CFG_ENV_SECT_SIZE */

#ifdef CFG_ENV_OFFSET_REDUND
	env_ptr->flags++;
	gd->env_valid ^= 3;		//1->2, 2->1
#endif

#if defined(CFG_ENV_SECT_SIZE) && (CFG_ENV_SECT_SIZE > CFG_ENV_SIZE)
	byteCnt = total;
	ret = nand_read(&nand_info[0], offset, &total,env_buffer);
	if (total < byteCnt) {
		printf("Error: only 0x%x bytes read\n",total);
		memset(env_buffer[total],-1,byteCnt-total);
	}
	/* copy current environment to temporary buffer */
	memcpy(&env_buffer[sectOffset], env_ptr, CFG_ENV_SIZE);
	total = CFG_ENV_SECT_SIZE;
#endif	/* CFG_ENV_SECT_SIZE */

	puts ("Erasing Nand...");
	if (nand_erase(&nand_info[0],offset, total)) return 1;
	puts ("Writing to Nand... ");
	byteCnt = total;
	ret = nand_write(&nand_info[0], offset, &total, (u_char*) env_ptr);
		
	if (ret || total != byteCnt) return 1;

	puts ("done\n");
	return ret;
}
#endif /* CMD_SAVEENV */

void serial_waitTxComplete(void);

#ifdef CFG_ENV_OFFSET_REDUND
void env_relocate_spec (void)
{
	puts ("env_relocate_spec start REDUND\n");
#if !defined(ENV_IS_EMBEDDED)
	ulong total;
	int crc1_ok = 0, crc2_ok = 0;
	env_t *tmp_env2;

	total = CFG_ENV_SIZE;

	tmp_env2 = (env_t *) malloc(CFG_ENV_SIZE);

	if (tmp_env2) {
		printf("nand_read start %s,0x%p\n",nand_info[0].name,nand_info[0].read);
		nand_read(&nand_info[0], CFG_ENV_OFFSET, &total,(u_char*) env_ptr);
		puts ("nand_read done\n");
		serial_waitTxComplete();
		printf("nand_read redund start %s,0x%p\n",nand_info[0].name,nand_info[0].read);
		serial_waitTxComplete();
		nand_read(&nand_info[0], CFG_ENV_OFFSET_REDUND, &total,(u_char*) tmp_env2);
		puts ("nand_read done\n");
		serial_waitTxComplete();

		crc1_ok = (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc);
		crc2_ok = (crc32(0, tmp_env2->data, ENV_SIZE) == tmp_env2->crc);
	} else {
		puts ("env space alloc failed\n");
	}

	if(!crc1_ok && !crc2_ok) {
		printf("Env CRC error on both, use default\n");
		serial_waitTxComplete();
		use_default();
	} else {
		int i;
		if(crc1_ok && !crc2_ok) i = 1;
		else if(!crc1_ok && crc2_ok) i = 2;
		else {
			/* both ok - check serial */
			if( (env_ptr->flags == 0) && (tmp_env2->flags == 255) ) i = 1;
			else if ( (env_ptr->flags == 255) && (tmp_env2->flags == 0) ) i = 2;
			else if( env_ptr->flags >= tmp_env2->flags) i = 1;
			else i = 2;
		}
		gd->env_valid = i;
		if(i == 2) {
			memcpy(env_ptr,tmp_env2,CFG_ENV_SIZE);
		}
	}
	if (tmp_env2) free(tmp_env2);
#endif /* ! ENV_IS_EMBEDDED */
}
#else /* ! CFG_ENV_OFFSET_REDUND */
/*
 * The legacy NAND code saved the environment in the first NAND device i.e.,
 * nand_dev_desc + 0. This is also the behaviour using the new NAND code.
 */
void env_relocate_spec (void)
{
	puts ("env_relocate_spec start3\n");
#if !defined(ENV_IS_EMBEDDED)
	if (nand_info[0].name) {
		ulong total;
		int ret;

		total = CFG_ENV_SIZE;
		printf("nand_read start %s\n",nand_info[0].name);
		ret = nand_read(&nand_info[0], CFG_ENV_OFFSET, &total, (u_char*)env_ptr);
		puts ("nand_read done\n");
		if (ret || total != CFG_ENV_SIZE) {
			DEBUG (MTD_DEBUG_LEVEL3, "env read err: ret:%x total:%x expected total:%x\n",ret,total,CFG_ENV_SIZE);
		} else {
			int crc = crc32(0, env_ptr->data, ENV_SIZE); 
			if ( crc == env_ptr->crc) return;
			DEBUG (MTD_DEBUG_LEVEL3, "env read crc32 err, calc:%x, read:%x length:%x\n",crc,env_ptr->crc,ENV_SIZE);
		}
  	}
	return use_default();
#endif /* ! ENV_IS_EMBEDDED */
}
#endif /* CFG_ENV_OFFSET_REDUND */

#if !defined(ENV_IS_EMBEDDED)
static void use_default()
{
	puts ("*** Warning - bad CRC or NAND, using default environment\n\n");

	if (default_environment_size > CFG_ENV_SIZE){
		puts ("*** Error - default environment is too large\n\n");
		return;
	}

	memset (env_ptr, 0, sizeof(env_t));
	memcpy (env_ptr->data,
			default_environment,
			default_environment_size);
	env_ptr->crc = crc32(0, env_ptr->data, ENV_SIZE);
	gd->env_valid = 1;

}
#endif

#endif /* CFG_ENV_IS_IN_NAND */
