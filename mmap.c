#define _GNU_SOURCE

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
int main(){
	printf("1\n");
	int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	printf("fd = %i\n", fd);
	if(fd < 0)
		return -1;
	struct tpacket_req req;
	req.tp_block_size = 4096;
	req.tp_block_nr = 1;
	req.tp_frame_size = 512;
	req.tp_frame_nr = 1*4096/512;
	printf("1\n");
	setsockopt(fd, SOL_PACKET, PACKET_RX_RING, (void *) &req, sizeof(req));
	int size = req.tp_block_size * req.tp_block_nr;
 	char * area;
	printf("1\n");
	area = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(area == NULL)
		return -1;
 	struct pollfd pfd;
	pfd.fd = fd;
	pfd.revents = 0;
	pfd.events = POLLIN|POLLRDNORM|POLLERR;
	unsigned long status;
	struct tpacket_hdr * packet;

	/* Setup our ringbuffer */
	struct iovec *ring;
	ring=malloc(req.tp_frame_nr * sizeof(struct iovec));
	int i;
	for(i=0; i<req.tp_frame_nr; i++) {
		ring[i].iov_base=(void *)((long)area)+(i*req.tp_frame_size);
		ring[i].iov_len=req.tp_frame_size;
	}
	int z = 0;
	while(z < 8){
		printf("in while frame size %i area = %p\n", req.tp_frame_size, area);
		if (z == 0)
			packet =(struct tpacket_hdr *)(area + z * req.tp_frame_size);
		else
			packet =(struct tpacket_hdr *)(area + z * (req.tp_frame_size-2));
		printf("address of packet %p\n", packet);
		z++;
		status = packet->tp_status;
		printf("status = %i\n", status);
		int retval = 0;
		if (status == TP_STATUS_KERNEL)
			retval = poll(&pfd, 1, 5000);
		if(pfd.revents & POLLERR)
			printf("error\n");
		if(pfd.revents & POLLIN)
			printf("pollin\n");
		if(pfd.revents & POLLRDNORM)
			printf("POLLRDNORM\n");
		printf("pdf.revents = %i\n", pfd.revents);
		pfd.revents = 0;
		
		printf("retval = %i\n", retval);
		printf("packet->tp_len %i\n", packet->tp_len);	
		printf("packet->tp_snaplen %i\n", packet->tp_snaplen);	
		printf("packet->tp_mac %i\n", packet->tp_mac);	
		printf("packet->tp_net %i\n", packet->tp_net);	
		printf("packet->tp_sec %i\n", packet->tp_sec);	
		printf("packet->tp_usec %i\n", packet->tp_usec);	
		char * data = (char *)packet + packet->tp_mac;
	//	data = area+packet->tp_mac;
		int i = 0;
		for(i = 0; i < packet->tp_len; i++)//1000; i++)
			printf("%x ", data[i]);
		printf("\n\n");
	}
      	/* loop */
//	int i = 0;
//	for(i = 0; i < 1000; i++){
	//	if(i%100 == 0)
	//		sleep(1);
      	//	r = sendto(nl_sd, (char *)&req, req.n.nlmsg_len, 0,  
	//		  (struct sockaddr *) &nladdr, sizeof(nladdr));
	//	if(r > 0)
	//		printf("r > 0, r = %i, len %i\n", r, req.n.nlmsg_len);
	/*	if(r < 0)
			printf("r < 0, %i\n", r);
		if(r == 0)
			printf("r == 0\n");
*/
//	}

	printf("\n\nfinised sent\n\n");
//	sleep(1);
     //   if (sendto_fd(nl_sd, (char *) &req, req.n.nlmsg_len) < 0)
    //            err(1, "error sending message via Netlink\n");
     //  	pthread_join(loopThread, NULL);
 	printf("\n\nfinised join\n\n");

       //        close(nl_sd);
       
}






