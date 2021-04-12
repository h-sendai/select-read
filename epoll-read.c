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
#include "set_timer.h"
#include "my_signal.h"
#include "get_num.h"

host_info *host_list = NULL;
int debug = 0;
volatile sig_atomic_t has_alarm = 0;
struct timeval start_time;

int usage(void)
{
    char *message =
"Usage: ./epoll-read ip_address:port [ip_address:port ...]\n"
"./epoll-read 192.168.10.16:24 192.168.10.17:24\n";
    fprintf(stderr, message);
    return 0;
}

void sig_alarm(int signo)
{
    has_alarm = 1;
    return;
}

int print_status_header()
{
    host_info *p;
    fprintf(stderr, "time ");
    for (p = host_list; p != NULL; p = p->next) {
        fprintf(stderr, "%s ", p->ip_address);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "---------------------------------------------------------\n");
    return 0;
}

int print_status()
{
    host_info *p;
    struct timeval now;
    struct timeval elapse;
    gettimeofday(&now, NULL);
    timersub(&now, &start_time, &elapse);
    fprintf(stderr, "%ld.%06ld ", elapse.tv_sec, elapse.tv_usec);

    int total_read_bytes = 0;
    for (p = host_list; p != NULL; p = p->next) {
        total_read_bytes += p->read_bytes;
        double read_bytes_MB = (double) p->read_bytes / 1024.0 / 1024.0;
        fprintf(stderr, "%.3f ( %d ) ", read_bytes_MB, p->read_count);
        /* XXX */
        /* reset counter */
        p->read_bytes = 0;
        p->read_count = 0;
    }
    fprintf(stderr, "%.3f", total_read_bytes/1024.0/1024.0);
    fprintf(stderr, "\n");
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

    int so_rcvbuf = 0;
    while ( (ch = getopt(argc, argv, "dhr:")) != -1) {
        switch (ch) {
            case 'd':
                debug =1;
                break;
            case 'h':
                usage();
                exit(0);
            case 'r':
                so_rcvbuf = get_num(optarg);
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

    for (p = host_list; p != NULL; p = p->next) {
        if ( (p->sockfd = tcp_socket()) < 0) {
            errx(1, "socket create fail");
        }
        if (so_rcvbuf > 0) {
            if (set_so_rcvbuf(p->sockfd, so_rcvbuf) < 0) {
                errx(1, "set_so_rcvbuf fail");
            }
        }
        if (debug) {
            int rcvbuf = get_so_rcvbuf(p->sockfd);
            fprintf(stderr, "rcvbuf: %d\n", rcvbuf);
        }
    }

    my_signal(SIGALRM, sig_alarm);
    set_timer(1, 0, 1, 0);
    gettimeofday(&start_time, NULL);

    // print_status_header();

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
        if (debug) {
            fprintf(stderr, "%s port %d\n", p->ip_address, p->port);
        }
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN;
        ev.data.ptr = p;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, p->sockfd, &ev) < 0) {
            err(1, "XXX epoll_ctl at %s port %d", p->ip_address, p->port);
        }
    }

    for ( ; ; ) {
        if (has_alarm) {
            print_status();
            has_alarm = 0;
        }
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
