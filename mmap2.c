/*#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <linux/if_packet.h>
#include <linux/genetlink.h>
#include <linux/if_ether.h>
#include <sys/mman.h>
#include <features.h>
#include <poll.h>
#include <net/if.h>
#include <linux/sockios.h>
*/
#ifndef __linux__
#error "Are you loco? This is Linux only!"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define __USE_XOPEN
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <features.h> /* for the glibc version number */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h> /* The L2 protocols */
#endif
#include <string.h>
#include <netinet/in.h>
#include <signal.h>

#define PF_ANA 27
#define AF_ANA PF_ANA
#define SIGANA 44 /* we define our own signal, hard coded since SIGRTMIN is different in user and in kernel space */ 

#define ANA_PACKET_READY 8
#define USER_PID 9
#define WRITTEN 1
#define READ 0
long long TOTAL = 100000000;

int fd=-1;
char *map;
struct tpacket_req req;
struct iovec *ring;

double packets= 0, polls= 1e-6, bytes= 0;
struct timeval t_start, t_end;

int received = 0;
void receiveData(int n, siginfo_t *info, void *unused){
	int i;

	/*somehow the value is not sent correctly..., so for now we use code to indicate the fragment nr */
	//	printf("###received data frame nr%i % value %i###\n", i, info->si_value.sival_int);
	//	printf("signr %i, sending pid %i , int %i\n", info->si_signo, info->si_pid, info->si_int);
	//	printf("receive from frame nr %i, \n", info->si_code);
		i = info->si_code;
	//	while(*(unsigned long*)ring[i].iov_base) { /* there is a packet, loop until all packets processed*/
			struct tpacket_hdr *h=ring[i].iov_base;
			struct sockaddr_ll *sll=(void *)h + TPACKET_ALIGN(sizeof(*h));
			unsigned char *bp=(unsigned char *)h + h->tp_mac; /* the captured packet starts here*/
			char * tmp = (char *)h;
			char * res = tmp + h->tp_net;
	//		printf("packet received: %s\n", res);
			packets++;
			bytes+= h->tp_len;
		
			/* "free this packet for further use, e.g. sender can overwrite it */
			h->tp_status = READ;
			received++;
			if(received == TOTAL){
				gettimeofday(&t_end, NULL);
				long long start = 1000000 * t_start.tv_sec + t_start.tv_usec;
				long long end = 1000000 * t_end.tv_sec + t_end.tv_usec;
			//	int secs = t_end.tv_sec - t_start.tv_sec;
			//	int usecs = t_end.tv_usec - t_start.tv_usec;
				long long delta = end - start;
				printf("totusec %lli, total %lli\n", delta, TOTAL);
			//	long double rate = ((double)TOTAL / (double)delta); /// 1000000;
			//	printf("received all packets in %i sec and %i microsec\n", secs, usecs);
			//	printf("packet rate Packets/sec: %ld\n", rate);
			}
		//	i=(i==req.tp_frame_nr-1) ? 0 : i+1; /* just look whether there is already an other packet avaiable */
//		}

}
int main ( int argc, char **argv )
{
	struct pollfd pfd;
	int i;
	char opt;
	int before= 0, after= 0, print= 0;
 
	/* setup the signal handler for SIGANA, 
 	 * SIGANA is received when a new packet is available
 	 */
	struct sigaction sig;
	sig.sa_sigaction = receiveData;
	sig.sa_flags = SA_SIGINFO;
	sigaction(SIGANA, &sig, NULL);


	/* Open the packet socket */
	print = 1;
	printf("Creating socket\n");
	if ( (fd=socket(PF_ANA, SOCK_DGRAM, 0))<0 ) {
		perror("socket()");
		return 1;
	}

	/* Create the buffer in kernel space */
	printf("Calling setsockopt PACKET_RX_RING\n");
	req.tp_block_size=4096;
	req.tp_frame_size=1024;
	req.tp_block_nr=4;
	req.tp_frame_nr=4*4;
	if ( (setsockopt(fd, SOL_PACKET, PACKET_RX_RING, (char *)&req,sizeof(req))) != 0 ) {
		perror("setsockopt(PACKET_RX_RING)");
		close(fd);
		return 1;
	};

	/* mmap() the memory */
	fprintf(stderr, "Mmaping ring buffer\n");
	map=mmap(NULL, req.tp_block_size * req.tp_block_nr, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED, fd, 0);
	if ( map==MAP_FAILED ) {
		perror("mmap()");
		close(fd);
		return 1;
	}

	/* Setup our ringbuffer */
	ring=malloc(req.tp_frame_nr * sizeof(struct iovec));
	for(i=0; i<req.tp_frame_nr; i++) {
		ring[i].iov_base=(void *)((long)map)+(i*req.tp_frame_size);
		ring[i].iov_len=req.tp_frame_size;
	}

	/* send the pid to the kernel on which we want to receive the echos */	
	int pid = getpid();
	if ( (setsockopt(fd, SOL_PACKET, USER_PID, (char *)&pid, sizeof(pid))) != 0 ) {
		perror("setsockopt(USER_PID)");
		close(fd);
		return 1;
	};

	/* while true, send the messages */
	i = 0;
	int z = 0;
	gettimeofday(&t_start, NULL);
	for(z = 0; z < TOTAL; z++) { 
	//	printf("send to frame nr: %i\n", i);
		struct tpacket_hdr *h=ring[i].iov_base;
		while(h->tp_status == WRITTEN){ /* memory ring full */
			printf("error: ring is full\n");
			sleep(1); /*just wait */
		}
		struct sockaddr_ll *sll=(void *)h + TPACKET_ALIGN(sizeof(*h));
		int netoff = TPACKET_ALIGN(TPACKET_HDRLEN);
		char * msg = "hello world";
		char * tmp = (char *)h;
		memcpy(tmp + netoff, "hello world", 12);
		
		h->tp_len = 12; /* the length */
		h->tp_snaplen = 0;
		h->tp_mac = 0;
		h->tp_net = netoff; /* offset where the message starts */
		h->tp_status = WRITTEN; /* we have written something in this memory location */

		/* the packet is now in memory -> inform the kernel, that there is a new packet
 		 * we cheat here: instead of putting the frame index in the value field, we set value to NULL
 		 * but the value len to the frame index
 		 */
		if ( (setsockopt(fd, SOL_PACKET, ANA_PACKET_READY, NULL, i)) != 0 ) {
			perror("setsockopt(ANA_PACKET_READY)");
			close(fd);
			return -1;
		}
		i=(i==req.tp_frame_nr-1) ? 0 : i+1;

	//	sleep(1);
	}

	return 0;
}



