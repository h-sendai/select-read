#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "my_socket.h"
#include "host_info.h"

host_info *host_list = NULL;
int debug = 0;

int usage(void)
{
	char *message =
"./sample ip_address:port [ip_address:port ...]\n";

	fprintf(stderr, message);
	return 0;
}


int main(int argc, char *argv[])
{
	int i, n, ch;
	host_info *p;
	int timeout = 2;
	int n_server;

	int epfd;
	int nfds;
	struct epoll_event ev, *ev_ret;

	while ( (ch = getopt(argc, argv, "d")) != -1) {
		switch (ch) {
			case 'd':
				debug =1;
				break;
			default:
				break;
		}
	}

	argc -= optind;
	argv += optind;
			
	if (argc == 0) {
		usage();
		exit(1);
	}

	n_server = argc;

	/* Prepare host_list */
	for (i = 0; i < argc; i++) {
		if (debug) {
			printf("%s\n", argv[i]);
		}
		host_list = addend(host_list, new_host(argv[i]));
	}

	/* get sockfd and connect to server */
	//for (p = host_list; p != NULL; p = p->next) {
	//	if (debug) {
	//		printf("try to connect: %s Port: %d\n", p->ip_address, p->port);
	//	}
	//	connect_to_server(p, timeout);
	//}

	for (p = host_list; p != NULL; p = p->next) {
		if ( (p->sockfd = tcp_socket()) < 0) {
			errx(1, "socket create fail");
		}
	}

	for (p = host_list; p != NULL; p = p->next) {
		if (connect_tcp(p->sockfd, p->ip_address, p->port) < 0) {
			errx(1, "connect to %s fail", p->ip_address);
		}
	}

	/* EPOLL Data structure */
	if ( (ev_ret = malloc(sizeof(struct epoll_event) * n_server)) == NULL) {
		err(1, "malloc for epoll_event data structure");
	}
	if ( (epfd = epoll_create(n_server)) < 0) {
		err(1, "epoll_create");
	}
	for (p = host_list; p != NULL; p = p->next) {
		fprintf(stderr, "%s port %d\n", p->ip_address, p->port);
		memset(&ev, 0, sizeof(ev));
		ev.events = EPOLLIN;
		ev.data.ptr = p;
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, p->sockfd, &ev) < 0) {
			err(1, "XXX epoll_ctl at %s port %d", p->ip_address, p->port);
		}
	}

	for ( ; ; ) {
		nfds = epoll_wait(epfd, ev_ret, n_server, timeout * 1000);
		if (nfds < 0) {
			if (errno == EINTR) {
				continue;
			}
			else {
				err(1, "epoll_wait");
			}
		}
		else if (nfds == 0) {
			fprintf(stderr, "epoll_wait timed out: %d sec\n", timeout);
			continue;
		}

		for (i = 0; i < nfds; i++) {
			p = ev_ret[i].data.ptr;
			n = read(p->sockfd, p->buf, p->bufsize);
			if (n < 0) {
				err(1, "read error");
			}
			else if (n == 0) {
				epoll_ctl(epfd, EPOLL_CTL_DEL, p->sockfd, NULL);
				if (close(p->sockfd) < 0) {
					err(1, "close on %s", p->ip_address);
				}
				n_server --;
				if (n_server == 0) {
					exit(0);
				}
			}
			else {
				p->read_bytes += n;
				p->read_count ++;
                if (debug) {
                    if ((p->read_count % 1000) == 0) {
                        fprintf(stderr, "%s port %d: %d bytes\n",
                            p->ip_address, p->port, p->read_bytes);
                    }
                }
			}
		}
	}
    return 0;
}
