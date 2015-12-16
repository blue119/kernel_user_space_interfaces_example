#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFSIZE 255
void Die(char *mess) { perror(mess); exit(1); }
	int sock;
	struct sockaddr_in echoserver;


	int receiveLoop(void){
		int i = 0;
		struct sockaddr_in echoclient;
		int clientlen, received, echolen;
		char buffer[BUFFSIZE];
		printf("----------\n");
		/*receive reply message*/
		while(i < 300){
			i++;
			/* Receive the word back from the server */
		//	fprintf(stderr, "Received: ");
			clientlen = sizeof(echoclient);
			if ((received = recvfrom(sock, buffer, BUFFSIZE, 0,
							(struct sockaddr *) &echoclient,
							&clientlen)) < 0) {
				Die("Failed to receive message");
			}
			/* Check that client and server are using same socket */
			if (echoserver.sin_addr.s_addr != echoclient.sin_addr.s_addr) {
				Die("Received a packet from an unexpected server");
			}
			buffer[received] = '\0';        /* Assure null terminated string */
		//	fprintf(stdout, buffer);
		//	fprintf(stdout, "\n");
		}
		close(sock);
		exit(0);

		return 1;

	}

int main(int argc, char *argv[]) {
	unsigned int echolen, clientlen, serverlen;
	int received = 0;

	/* Create the UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		Die("Failed to create socket");
	}


	      /* Construct the server sockaddr_in structure */
          memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
          echoserver.sin_family = AF_INET;                  /* Internet/IP */
          echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any IP address */
          echoserver.sin_port = htons(9999);       /* server port */

          /* Bind the socket */
          serverlen = sizeof(echoserver);
          if (bind(sock, (struct sockaddr *) &echoserver, serverlen) < 0) {
            Die("Failed to bind server socket");
          }
	/* Construct the server sockaddr_in structure */
	memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
	echoserver.sin_family = AF_INET;                  /* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr("127.0.0.1");  /* IP address */
	echoserver.sin_port = htons(5555);       /* server port */
	
	pthread_t loopThread;
	int ret = pthread_create(&loopThread, NULL, (void *) receiveLoop, NULL);
	if (ret < 0) {
		printf("Problem creating data listenLoop thread \n");
		return 1;
	} 
	
	/* Send the word to the server */
	char * buf = "hello world";
	echolen = strlen(buf);
	int i;
	for(i = 0; i < 300; i++){
	if (sendto(sock, buf, echolen, 0,
				(struct sockaddr *) &echoserver,
				sizeof(echoserver)) != echolen) {
		Die("Mismatch in number of sent bytes");
	}
	}
	pthread_join(loopThread, NULL);
	return 0;
}
