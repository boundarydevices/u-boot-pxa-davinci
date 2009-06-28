/*
 * Windows CE load support
 *
 * Eric Nelson <eric.nelson@boundarydevices.com> 2009
 *
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <rtc.h>
#include "tftp.h"
#include <zlib.h>

#if defined(CONFIG_CMD_NET) && defined(CONFIG_CMD_CELOAD)

enum load_state_t {
	STATE_SEND_BOOTME,
	STATE_RX_DATA,
};

static enum load_state_t load_state ;

struct ack_packet {
	ushort	ack ;
	ushort	block ;
};

static struct ack_packet tx_ack ;
static ushort server_port ;
static ulong  server_ip ;
static int do_adler ;
static uLong adler_val ;
unsigned ce_load_max ;
unsigned ce_load_min ;

#define TFTP_PACKET_SIZE	512
#define EDBG_PORT	980
#define EDBG_ID		"EDBG"
#define	EDBG_SERVICE_BOOT	0xff
#define EDBG_FLAGS_REQUEST	0x01
#define EDBG_CMD_BOOTME		0x00
#define BOOTME_VER_MAJOR	0x03
#define BOOTME_VER_MINOR	0x03
#define CPUID_PXA270		0x41
#define BOOTME_VER		0x02
#define BOOTME_FLAGS		0

struct ce_boot_header {
	ulong	id ; 		// "EDBG"
	uchar	service ;
	uchar	flags ;
	uchar	sequence ;
	uchar	cmd ;
};

struct ce_bootme_data {
	uchar	major ;
	uchar	minor ;
	uchar	mac[6];
	ulong	ip ;
	char	platform[17];
	char	device_name[17];
	uchar	cpu_id ;
	uchar	bootme_ver ;
	ulong 	flags ;
	ushort	download_port ;
	ushort	svc_port ;
};

struct ce_bootme_packet {
	struct ce_boot_header	header ;
	struct ce_bootme_data	data ;
};

static void
ce_load_send (void)
{
	struct ce_bootme_packet pkt;

	printf ("%s\n", __FUNCTION__);

	memset (&pkt, 0, sizeof(pkt));

	memcpy(&pkt.header.id, EDBG_ID, sizeof(pkt.header.id) );
	pkt.header.service = EDBG_SERVICE_BOOT ;
	pkt.header.flags = EDBG_FLAGS_REQUEST ;
	pkt.header.sequence = 0 ;
	pkt.header.cmd = EDBG_CMD_BOOTME ;

	pkt.data.major = BOOTME_VER_MAJOR ;
	pkt.data.minor = BOOTME_VER_MINOR ;
	memcpy(pkt.data.mac,NetOurEther, sizeof(pkt.data.mac));
	memcpy(&pkt.data.ip,&NetOurIP,sizeof(pkt.data.ip));
	strcpy(pkt.data.platform, "MyPlatform");
	strcpy(pkt.data.device_name, "MyDevice");
	pkt.data.cpu_id = CPUID_PXA270 ;
	pkt.data.bootme_ver = BOOTME_VER ;
	pkt.data.flags = BOOTME_FLAGS ;
	pkt.data.download_port = 0 ;
	pkt.data.svc_port = 0 ;

	memcpy ((char *)NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE, (char *)&pkt, sizeof(pkt));

	NetSendUDPPacket (NetServerEther, 0xFFFFFFFF, EDBG_PORT, EDBG_PORT, sizeof(pkt));
}

static int retries ;

static void
ce_load_timeout (void)
{
	puts ("Timeout\n");
	if( STATE_SEND_BOOTME == load_state ){
		if( 4 <= ++retries )
			NetState = NETLOOP_FAIL;
		else
			ce_load_send ();
	} else {
			NetState = NETLOOP_FAIL;
	}
	return;
}

extern IPaddr_t NetSenderIP ;
extern uchar    NetSenderMac[6];

static void
ce_load_handler (uchar *pkt, unsigned dest, unsigned src, unsigned len)
{
//	printf ("%s: packet received: port 0x%x, %u bytes\n", __FUNCTION__, dest, len );
	if(EDBG_PORT != dest)
		return ;
	if( getenv("dumppackets") ){
		int i ;
		uchar const *next = pkt ;
		for( i = 0 ; i < len ; i++ ){
			int rem = i&0x0f;
			if( 8 == rem )
				printf( "  " );
			else if( 0 == rem )
				printf( "\n" );
	
			printf( "%02x ", *next++ );
		}
		printf( "\n" );
	}

	if( STATE_SEND_BOOTME == load_state ){
		if( 6 <= len ){
			ushort op ;
			memcpy(&op, pkt, sizeof(op));
			op = ntohs(op);
			if( TFTP_WRQ == op ){
				char *next = (char *)pkt + 2 ;
				char *filename = next ;
				pkt[len] = '\0' ; // just in case
				next += strlen(next);
				if( next < (char *)pkt+len ){
					char senderIP[32];
                                        tx_ack.ack = htons(TFTP_ACK);
					tx_ack.block = 0 ;
					load_state = STATE_RX_DATA ;
                                        server_port = src ;
					memcpy ((char *)NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE, (char *)&tx_ack, sizeof(tx_ack));
                                        ip_to_string(NetSenderIP,senderIP);
printf( "%s: receiving %s from sender %s\n", __func__, filename, senderIP );
                                        NetSendUDPPacket(NetSenderMac, NetSenderIP, src, EDBG_PORT, sizeof(tx_ack));
                                        NetSetTimeout (5 * CFG_HZ, ce_load_timeout);
				}
				else
					printf( "%s: Error in WRQ request\n", __func__ );
			}
			else
				printf( "%s: rx %04x %02x %02x in state %u\n", __func__, op, pkt[0], pkt[1], load_state );
		}
		else
			printf( "%s: too short %u\n", __func__, len );
	} else {
		ushort op ;
		memcpy(&op, pkt, sizeof(op));
		op = ntohs(op);
		if( TFTP_DATA == op ){
			ushort prev_block = ntohs(tx_ack.block);
			ushort block ;
			memcpy(&block,pkt+sizeof(op),sizeof(block));
			block = ntohs(block);
//			printf( "rx block %u (after %u) from port %u\n", block, tx_ack.block, src );
			if( prev_block + 1 == block ){
				unsigned offs = prev_block*TFTP_PACKET_SIZE ;
				unsigned data_len = len-sizeof(op)-sizeof(block);
				if( offs + data_len <= ce_load_max ){
					uchar *data_start = pkt+sizeof(op)+sizeof(block);
					uchar *out_start = (uchar *)load_addr + offs ;
					memcpy(out_start,data_start,data_len);
					if( do_adler ){
						adler_val = adler32( adler_val, (Bytef *)data_start,data_len);
					}
					tx_ack.ack = htons(TFTP_ACK);
					tx_ack.block = htons(block);
					if( TFTP_PACKET_SIZE > data_len ){
						unsigned long ram_adler = 0 ;
						unsigned byteCount = offs + data_len ;
						printf( "%s: done after %u blocks, %u bytes", __func__, block, byteCount );
						if( do_adler ){
							printf( ", adler 0x%08lx\n", adler_val );
                                                        ram_adler = adler32(0,(Bytef *)load_addr,byteCount);
							printf( "   adler(0x%x).%u == 0x%08lx\n", load_addr, byteCount, ram_adler );
						} else
							printf( "\n" );
						if( byteCount >= ce_load_min ){
							if( do_adler && ( ram_adler != adler_val ) ){
								printf( "adler mismatch: 0x%08lx != 0x%08lx\n", ram_adler, adler_val );
								NetState = NETLOOP_FAIL ;
							} else
								char buf [12];
								sprintf(buf, "%p", load_addr);
								setenv("loadaddr", buf);
								sprintf(buf, "0x%lX", byteCount);
								setenv("filesize", buf);
                                                                NetState = NETLOOP_SUCCESS;
						} else {
							printf( "%s: less than minimum of %u bytes\n", __func__, ce_load_min );
                                                        NetState = NETLOOP_FAIL;
						}
					} // done
				} else {
					printf( "%s: max file size of %u exceeded\n", __func__, ce_load_max );
                                        NetState = NETLOOP_FAIL;
				}
			}
			else
				printf( "%s: expected block %u, got %u\n", __func__, ntohs(tx_ack.block)+1, block );
			memcpy ((char *)NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE, (char *)&tx_ack, sizeof(tx_ack));
		}
		else
			printf( "%s: rx %04x %02x %02x in state %u\n", __func__, op, pkt[0], pkt[1], load_state );
		NetSendUDPPacket(NetSenderMac, NetSenderIP, src, EDBG_PORT, sizeof(tx_ack));
		NetSetTimeout (2 * CFG_HZ, ce_load_timeout);
	}
}

void
CeLoadStart (void)
{
	printf ("%s\n", __FUNCTION__);
	NetSetTimeout (2 * CFG_HZ, ce_load_timeout);
	NetSetHandler(ce_load_handler);
	memset (NetServerEther, 0, 6);

	do_adler = (0 != getenv("ceload_adler"));
        adler_val = 0 ;
        load_state = STATE_SEND_BOOTME ;
	retries = 0 ;
	ce_load_send ();
}

#endif
