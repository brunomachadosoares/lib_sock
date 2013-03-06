CC     = gcc

CFLAGS = -g -Wall -O2 --pedantic \
		`pkg-config glib-2.0 --cflags`

LIBS 	= `pkg-config glib-2.0 --libs`

OBJS 	= client.c server.c log.c global.c

all:
	$(CC) -c $(OBJS) $(LIBS) $(CFLAGS)
	ld -shared -o libsock.so -lc server.o client.o log.o global.o
	@cp libsock.so libsock.h ../bin

clean:
	rm -rf *.o *.so
