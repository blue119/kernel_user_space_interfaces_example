#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFSIZE 5096 

int main(int argc, char *argv[]) {
	int sendlen, receivelen;
	int received = 0;
	char buffer[BUFFSIZE];
	struct sockaddr_in receivesocket;
	struct sockaddr_in sendsocket;
	int sock;
	
	int ret = 0;

	/* Create the UDP socket */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket");
		return -1;
	}

	/* my address */
	memset(&receivesocket, 0, sizeof(receivesocket));  
	receivesocket.sin_family = AF_INET; 
	receivesocket.sin_addr.s_addr = htonl(INADDR_ANY);
	receivesocket.sin_port = htons(9999);

	receivelen = sizeof(receivesocket);
	if (bind(sock, (struct sockaddr *) &receivesocket, receivelen) < 0) {
		perror("bind");
		return -1;		
	}

	/* kernel address */
	memset(&sendsocket, 0, sizeof(sendsocket));
	sendsocket.sin_family = AF_INET;
	sendsocket.sin_addr.s_addr = inet_addr("127.0.0.1");
	sendsocket.sin_port = htons(5555);

	/* Send message to the server */
	memcpy(buffer, "hello world", strlen("hello world") + 1);
	sendlen = strlen(buffer) + 1;
	
	if (sendto(sock, buffer, sendlen, 0, (struct sockaddr *) &sendsocket, sizeof(sendsocket)) != sendlen) {
		perror("sendto");
		return -1;
	}

	memset(buffer, 0, BUFFSIZE);
	if ((received = recvfrom(sock, buffer, BUFFSIZE, 0, NULL, NULL)) < 0){
		perror("recvfrom");
		return -1;
	}
	
	printf("message received: %s\n", buffer);

	return 0;
}
