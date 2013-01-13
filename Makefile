all: mkdir server client

CFLAGS=-g3

mkdir:
	test -d ./build || mkdir ./build

client: 
	gcc $(CFLAGS) logger.c  client.c -o ./build/client.bin -lpthread -lcurses -DSERVER_ACTIVE -lconfig

server:
	gcc $(CFLAGS) logger.c  server.c -o ./build/server.bin -lpthread -lcurses -lconfig

clean:
	rm -rf *.o  ./build

clean_all: clean
	rm -rf *.out *~ 
