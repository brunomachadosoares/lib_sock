CC      = gcc
CFLAGS  = -g -Wall -O2 \
			`pkg-config glib-2.0 --cflags` \
			-I../src/ \
			-L../src/ 

LIBS    = -lsock \
			`pkg-config glib-2.0 --libs` 

BIN     = server_example
OBJS    = server.o

all:$(BIN)

$(BIN): $(HEADERS) $(OBJS) Makefile
	$(CC) $(OBJS) -o $@ $(LIBS) $(CFLAGS)

clean:
	rm -rf $(BIN) *.o
