#include "my_socket.h"

int tcp_socket(void)
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

int udp_socket(void)
{
	return socket(AF_INET, SOCK_DGRAM, 0);
}

int connect_tcp_timeout(int sockfd, char *ip_address, int port, int timeout)
{
	fprintf(stderr, "not yet implement\n");
	exit(1);
}

/* from kolc */
int connect_tcp(int sockfd, char *host, int port)
{
	struct sockaddr_in servaddr;
	struct sockaddr_in *resaddr;
	struct addrinfo    hints;
	struct addrinfo    *res;
	int err;

	res = 0;
	memset((char *)&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	if ( (err = getaddrinfo(host, 0, &hints, &res)) != 0) {
		return -1;
	}

	resaddr = (struct sockaddr_in *)res->ai_addr;
	memset((char *)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(port);
	servaddr.sin_addr   = resaddr->sin_addr;
	freeaddrinfo(res);

	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("connect");
		return -1;
	}
	//return connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	return 0;
}
