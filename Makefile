all: server client

client: 
	gcc client.c -o ./build/client.bin -lpthreads -lcurses

server:
	gcc client.c -o ./build/client.bin -lpthreads -lcurses

clean:
	rm -rf *.o server.bin client.bin

clean_all:
	rm -rf *.out *.o *~  server.bin client.bin
