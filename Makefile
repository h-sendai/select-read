PROG = sample
CFLAGS = -g -O0 -Wall

all: ${PROG}

${PROG}: my_socket.o host_info.o sample.o

clean:
	rm -f *.o ${PROG}
