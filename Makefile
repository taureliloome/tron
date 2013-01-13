all: mkdir server client

mkdir: ./build
	test -d ./build || mkdir ./build

client: 
	gcc client.c -o ./build/client.bin -lpthread -lcurses

server:
	gcc server.c -o ./build/server.bin -lpthread -lcurses

clean:
	rm -rf *.o server.bin client.bin

clean_all:
	rm -rf *.out *.o *~  server.bin client.bin
