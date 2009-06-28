/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>

extern int do_bootm (cmd_tbl_t *, int, int, char *[]);

static int netboot_common (proto_t, cmd_tbl_t *, int , char *[]);

int do_bootp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common (BOOTP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	bootp,	3,	1,	do_bootp,
	"bootp\t- boot image via network using BootP/TFTP protocol\n",
	"[loadAddress] [bootfilename]\n"
);

extern int get_rom_mac (char *v_rom_mac);
extern int set_rom_mac (char const *v_rom_mac);

/*
 * returns -1 if not valid hex
 */
static int hexValue( char c )
{
	if( ( '0' <= c ) && ( '9' >= c ) )
	{
		return c-'0' ;
	}
	else if( ( 'A' <= c ) && ( 'F' >= c ) )
	{
		return c-'A'+10 ;
	}
	else if( ( 'a' <= c ) && ( 'f' >= c ) )
	{
		return c-'a'+10 ;
	}
	else
		return -1 ;
}

// returns non-zero to indicate success
int parse_mac( char const *macString, // input
               char       *macaddr )  // output: not NULL-terminated
{
	int i ;
	for( i = 0 ; i < 6 ; i++ )
	{
		char high, low, term ;
		int  highval, lowval ;
		high = *macString++ ;

		if( ( 0 == high )
		    ||
		    ( 0 > ( highval = hexValue(high) ) ) )
			break ;
		low  = *macString++ ;
		if( ( 0 == low )
		    ||
		    ( 0 > ( lowval = hexValue(low) ) ) )
			break ;

		term = *macString++ ;
		if( 5 > i )
		{
			if( ( '-' != term ) && ( ':' != term ) )
				break ;
		}
		else if( '\0' != term )
			break ;

		*macaddr++ = (highval<<4) | lowval ;
	}

	return ( 6 == i );
}

int do_mac (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
   int rval = -1 ;

	if( 2 == argc )
	{
		char mac[6];
		if( parse_mac( argv[1], mac ) )
		{
			printf( "setting mac address to %02x:%02x:%02x:%02x:%02x:%02x\n",
				mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] );
			if( set_rom_mac( mac ) ){
				printf( "done\n" );
				rval = 0 ;
			}
			else
				printf( "Error setting mac address\n" );
		}
		else
			printf( "Error parsing mac: use form NN:NN:NN:NN:NN:NN\n" );
	}
	else
	{
		char mac[6];
		if( get_rom_mac( mac ) ){
			printf( "mac address %02x:%02x:%02x:%02x:%02x:%02x\n",
				mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] );
			rval = 0 ;
		}
		else if( 0xFF == mac[0] )
			printf( "MAC has not been programmed\n" );
		else
			printf( "error reading mac\n" );
	}
	return rval ;
}

U_BOOT_CMD(
	mac,	3,	1,	do_mac,
	"mac\t- read/write mac address\n",
	"- supply a parameter of the form NN:NN:NN:NN:NN:NN to set"
);


int do_tftpb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common (TFTP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	tftpboot,	3,	1,	do_tftpb,
	"tftpboot- boot image via network using TFTP protocol\n",
	"[loadAddress] [[hostIPaddr:]bootfilename]\n"
);

int do_rarpb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common (RARP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	rarpboot,	3,	1,	do_rarpb,
	"rarpboot- boot image via network using RARP/TFTP protocol\n",
	"[loadAddress] [bootfilename]\n"
);

#if defined(CONFIG_CMD_DHCP)
int do_dhcp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common(DHCP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	dhcp,	3,	1,	do_dhcp,
	"dhcp\t- invoke DHCP client to obtain IP/boot params\n",
	"\n"
);
#endif

#if defined(CONFIG_CMD_NFS)
int do_nfs (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return netboot_common(NFS, cmdtp, argc, argv);
}

U_BOOT_CMD(
	nfs,	3,	1,	do_nfs,
	"nfs\t- boot image via network using NFS protocol\n",
	"[loadAddress] [[hostIPaddr:]bootfilename]\n"
);
#endif

static void netboot_update_env (void)
{
	char tmp[22];

	if (NetOurGatewayIP) {
		ip_to_string (NetOurGatewayIP, tmp);
		setenv ("gatewayip", tmp);
	}

	if (NetOurSubnetMask) {
		ip_to_string (NetOurSubnetMask, tmp);
		setenv ("netmask", tmp);
	}

	if (NetOurHostName[0])
		setenv ("hostname", NetOurHostName);

	if (NetOurRootPath[0])
		setenv ("rootpath", NetOurRootPath);

	if (NetOurIP) {
		ip_to_string (NetOurIP, tmp);
		setenv ("ipaddr", tmp);
	}

	if (NetServerIP) {
		ip_to_string (NetServerIP, tmp);
		setenv ("serverip", tmp);
	}

	if (NetOurDNSIP) {
		ip_to_string (NetOurDNSIP, tmp);
		setenv ("dnsip", tmp);
	}
#if defined(CONFIG_BOOTP_DNS2)
	if (NetOurDNS2IP) {
		ip_to_string (NetOurDNS2IP, tmp);
		setenv ("dnsip2", tmp);
	}
#endif
	if (NetOurNISDomain[0])
		setenv ("domain", NetOurNISDomain);

#if defined(CONFIG_CMD_SNTP) \
    && defined(CONFIG_BOOTP_TIMEOFFSET)
	if (NetTimeOffset) {
		sprintf (tmp, "%d", NetTimeOffset);
		setenv ("timeoffset", tmp);
	}
#endif
#if defined(CONFIG_CMD_SNTP) \
    && defined(CONFIG_BOOTP_NTPSERVER)
	if (NetNtpServerIP) {
		ip_to_string (NetNtpServerIP, tmp);
		setenv ("ntpserverip", tmp);
	}
#endif
}

static int
netboot_common (proto_t proto, cmd_tbl_t *cmdtp, int argc, char *argv[])
{
	char *s;
	int   rcode = 0;
	int   size;

	/* pre-set load_addr */
	if ((s = getenv("loadaddr")) != NULL) {
		load_addr = simple_strtoul(s, NULL, 16);
	}

	switch (argc) {
	case 1:
		break;

	case 2:	/* only one arg - accept two forms:
		 * just load address, or just boot file name.
		 * The latter form must be written "filename" here.
		 */
		if (argv[1][0] == '"') {	/* just boot filename */
			copy_filename (BootFile, argv[1], sizeof(BootFile));
		} else {			/* load address	*/
			load_addr = simple_strtoul(argv[1], NULL, 16);
		}
		break;

	case 3:	load_addr = simple_strtoul(argv[1], NULL, 16);
		copy_filename (BootFile, argv[2], sizeof(BootFile));

		break;

	default: printf ("Usage:\n%s\n", cmdtp->usage);
		show_boot_progress (-80);
		return 1;
	}

	show_boot_progress (80);
	if ((size = NetLoop(proto)) < 0) {
		show_boot_progress (-81);
		return 1;
	}

	show_boot_progress (81);
	/* NetLoop ok, update environment */
	netboot_update_env();

	/* done if no file was loaded (no errors though) */
	if (size == 0) {
		show_boot_progress (-82);
		return 0;
	}

	/* flush cache */
	flush_cache(load_addr, size);

	/* Loading ok, check if we should attempt an auto-start */
	if (((s = getenv("autostart")) != NULL) && (strcmp(s,"yes") == 0)) {
		char *local_args[2];
		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf ("Automatic boot of image at addr 0x%08lX ...\n",
			load_addr);
		show_boot_progress (82);
		rcode = do_bootm (cmdtp, 0, 1, local_args);
	}

#ifdef CONFIG_AUTOSCRIPT
	if (((s = getenv("autoscript")) != NULL) && (strcmp(s,"yes") == 0)) {
		printf ("Running autoscript at addr 0x%08lX", load_addr);

		s = getenv ("autoscript_uname");
		if (s)
			printf (":%s ...\n", s);
		else
			puts (" ...\n");

		show_boot_progress (83);
		rcode = autoscript (load_addr, s);
	}
#endif
	if (rcode < 0)
		show_boot_progress (-83);
	else
		show_boot_progress (84);
	return rcode;
}

#if defined(CONFIG_CMD_PING)
int do_ping (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2)
		return -1;

	NetPingIP = string_to_ip(argv[1]);
	if (NetPingIP == 0) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}

	if (NetLoop(PING) < 0) {
		printf("ping failed; host %s is not alive\n", argv[1]);
		return 1;
	}

	printf("host %s is alive\n", argv[1]);

	return 0;
}

U_BOOT_CMD(
	ping,	2,	1,	do_ping,
	"ping\t- send ICMP ECHO_REQUEST to network host\n",
	"pingAddress\n"
);
#endif

#if defined(CONFIG_CMD_CDP)

static void cdp_update_env(void)
{
	char tmp[16];

	if (CDPApplianceVLAN != htons(-1)) {
		printf("CDP offered appliance VLAN %d\n", ntohs(CDPApplianceVLAN));
		VLAN_to_string(CDPApplianceVLAN, tmp);
		setenv("vlan", tmp);
		NetOurVLAN = CDPApplianceVLAN;
	}

	if (CDPNativeVLAN != htons(-1)) {
		printf("CDP offered native VLAN %d\n", ntohs(CDPNativeVLAN));
		VLAN_to_string(CDPNativeVLAN, tmp);
		setenv("nvlan", tmp);
		NetOurNativeVLAN = CDPNativeVLAN;
	}

}

int do_cdp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int r;

	r = NetLoop(CDP);
	if (r < 0) {
		printf("cdp failed; perhaps not a CISCO switch?\n");
		return 1;
	}

	cdp_update_env();

	return 0;
}

U_BOOT_CMD(
	cdp,	1,	1,	do_cdp,
	"cdp\t- Perform CDP network configuration\n",
);
#endif

#if defined(CONFIG_CMD_SNTP)
int do_sntp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *toff;

	if (argc < 2) {
		NetNtpServerIP = getenv_IPaddr ("ntpserverip");
		if (NetNtpServerIP == 0) {
			printf ("ntpserverip not set\n");
			return (1);
		}
	} else {
		NetNtpServerIP = string_to_ip(argv[1]);
		if (NetNtpServerIP == 0) {
			printf ("Bad NTP server IP address\n");
			return (1);
		}
	}

	toff = getenv ("timeoffset");
	if (toff == NULL) NetTimeOffset = 0;
	else NetTimeOffset = simple_strtol (toff, NULL, 10);

	if (NetLoop(SNTP) < 0) {
		printf("SNTP failed: host %s not responding\n", argv[1]);
		return 1;
	}

	return 0;
}

U_BOOT_CMD(
	sntp,	2,	1,	do_sntp,
	"sntp\t- synchronize RTC via network\n",
	"[NTP server IP]\n"
);
#endif

#if defined(CONFIG_CMD_CELOAD)
 
extern unsigned ce_load_max ;
extern unsigned ce_load_min ;

int do_ceload (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i ;
	printf( "%s: %d args\n", __func__, argc );
	for( i = 0 ; i < argc ; i++ ){
		printf( "argv[%d] == %s\n", i, argv[i] );
	}
	if( 3 <= argc ){
		ce_load_max = simple_strtoul(argv[2],0,0);
		ce_load_min = 0 ;
		if( 3 < argc ){
                        ce_load_min = simple_strtoul(argv[3],0,0);
		}
                load_addr = simple_strtoul(argv[1], NULL, 16);
		if( ce_load_min <= ce_load_max ){
			if (NetLoop(CE_LOAD) >= 0) {
				printf( "success\n" );
				return 0 ;
			} else
				printf("ce_load failed: no host responding\n" );
		} else
                        printf ("Usage:\n%s\n", cmdtp->help);
	}
	else
                printf ("Usage:\n%s\n", cmdtp->help);

	return 1;
}

U_BOOT_CMD(
	ceload,	4,	0,	do_ceload,
	"ceload\t- load Windows CE via network\n",
	"loadAddress maxsize [minsize]\n"
	"Use this to load a Windows CE image through the platform builder\n"
	"protocol (BOOTME+TFTP server)\n"
);
#endif
