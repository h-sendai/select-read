#ifndef _SET_TIMER
#define _SET_TIMER 1

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

extern int set_timer(long sec, long usec, long sec_interval, long usec_interval);
extern struct timeval float2timeval(double x);
extern struct timeval str2timeval(char *str);
#endif

