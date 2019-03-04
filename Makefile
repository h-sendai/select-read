PROG = epoll-read
CFLAGS += -g -O2 -Wall

all: $(PROG)
OBJS += $(PROG).o
OBJS += host_info.o
OBJS += my_socket.o
OBJS += set_timer.o
OBJS += my_signal.o

$(PROG): $(OBJS)

clean:
	rm -f *.o $(PROG)
