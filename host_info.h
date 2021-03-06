#ifndef _HOST_INFO
#define _HOST_INFO

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

struct host_info_tag {
	char *ip_address;
	int   port;
	char *buf;
	int	  bufsize;
	int   sockfd;
	long  read_bytes;
	int   read_count;
	struct host_info_tag *next;
};
typedef struct host_info_tag host_info;
typedef struct sockaddr SA;

#define DEFAULT_PORT    24
#define DEFAULT_BUFSIZE 2*1024*1024; /*  2 MB */

/* Taken from "The Practice of Programming, Kernighan and Pike" */
extern host_info *new_host(char *host_and_port, int bufsize);
extern host_info *addfront(host_info *host_list, host_info *newp);
extern host_info *addend(host_info *host_list, host_info *newp);
extern int        connect_to_server(host_info *host_info, int timeout);

extern int debug;
#endif
