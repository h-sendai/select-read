#ifndef _MY_SOCKET
#define _MY_SOCKET

#include <sys/wait.h>
#include <sys/stat.h>    /* for S_xxx file mode constants */
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>     /* for iovec{} and readv/writev */
#include <sys/time.h>

#include <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>   /* inet(3) functions */

#include <err.h>
#include <errno.h>
#include <fcntl.h>       /* for nonblocking */
#include <netdb.h>		 /* for getaddrinfo() */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

extern int tcp_socket(void);
extern int udp_socket(void);
extern int connect_tcp_timeout(int sockfd, char *host, int port, int timeout);
extern int connect_tcp(int sockfd, char *host, int port);

#endif
