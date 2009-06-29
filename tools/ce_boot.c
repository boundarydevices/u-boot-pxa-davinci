#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/errno.h>

typedef unsigned char uchar ;
typedef unsigned short ushort ;
typedef unsigned long ulong ;

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


// rval = execve( args[0], (char **)args, environ );
//  atftp -p -l ~/NK5-Hydrogen.nb0 192.168.0.6 980

static void showArgs( char const **args ){
	int i = 0 ;
	while( *args ){
		printf( "arg[%d] == %s\n", i++, *args++ );
	}
}

static int startTransfer( char const **args ){
	int child ;
	showArgs(args);
   
	child = fork();
	if( 0 == child ){
	      execv( args[0], (char **)args );
	      exit(0);
	}
	return child ;
}

static void childHandler
   ( int             sig, 
     struct siginfo *info, 
     void           *context )
{
   if( SIGCHLD == sig )
   {
      int exitCode ;
      struct rusage usage ;

      int pid ;
      while( 0 < ( pid = wait3( &exitCode, WNOHANG, &usage ) ) )
      {
          printf( "child process %d died\n", pid );
      }
   }
}

int main( int argc, char const * const argv[] )
{
	if( 2 > argc ){
		fprintf( stderr, "Usage: %s fileName\n", argv[0] );
		return -1 ;
	}
        int const fd = socket( AF_INET, SOCK_DGRAM, 0 );
        int doit = 1 ;
        int err = setsockopt( fd, SOL_SOCKET, SO_BROADCAST, &doit, sizeof(doit) );
	if( err ){
		perror( "set SO_BROADCAST" );
	}
	doit = 0x44 ;
        int doitlen = sizeof( doit );
	err = getsockopt(fd,SOL_SOCKET, SO_BROADCAST, &doit, &doitlen );
	if( err )
		perror( "get SO_BROADCAST" );
	else
		printf( "broadcast: %d, len %d\n", doit, doitlen );

        struct sockaddr_in local ;
        local.sin_family      = AF_INET ;
        local.sin_addr.s_addr = INADDR_ANY ;
        local.sin_port        = htons(980);
        err = bind( fd, (struct sockaddr *)&local, sizeof( local ) );
	if( err )
		perror( "bind" );
        
   
	struct sigaction sa;
 
	/* Initialize the sa structure */
	sa.sa_handler = (__sighandler_t)childHandler ;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO ; // pass info
	
	sigaction( SIGCHLD, &sa, 0 );

        while (1) {
		char inBuf[512];
                int numRead ;
		struct sockaddr_in rem ;
		socklen_t remSize = sizeof(rem);
		if( 0 < ( numRead = recvfrom( fd, inBuf, sizeof(inBuf), 0, (struct sockaddr *)&rem, &remSize ) ) ) {
			char cDeviceIP[36] = { 0 };
			char cPortNum[12] = { 0 };
			strcpy( cDeviceIP, inet_ntoa(rem.sin_addr) );
			snprintf(cPortNum,sizeof(cPortNum)-1,"%u", ntohs(rem.sin_port) );
			printf( "rx %u bytes from %s, port %s (0x%x)\n", numRead, cDeviceIP, cPortNum, ntohs(rem.sin_port) );
			if( sizeof(struct ce_bootme_packet) <= numRead ){
				int child ;
				char const *cmdline[7] = {
					"/usr/bin/atftp"
				,	"-p"
				,	"-l"
				,	argv[1]
				,	cDeviceIP
				,	cPortNum
				,	0
				};
				struct ce_bootme_packet *pkt = (struct ce_bootme_packet *)inBuf ;
				printf( "platform <%s>, device <%s>\n", pkt->data.platform, pkt->data.device_name );
				child = startTransfer( cmdline );
				printf( "child process %d\n", child );
			}
		}
		else if( EINTR != errno ) // EINTR is expected when children die 
			fprintf( stderr, "recvfrom:%d:%m\n", errno );
        }

        return 0 ;
}

