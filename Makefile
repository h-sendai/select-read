PROG = sample
CFLAGS += -g -O2 -Wall

all: $(PROG)
OBJS += $(PROG).o
OBJS += host_info.o
OBJS += my_socket.o
OBJS += set_timer.o

$(PROG): $(OBJS)

clean:
	rm -f *.o $(PROG)
