all: mkdir server client

mkdir:
	test -d ./build || mkdir ./build

client: 
	gcc client.c -o ./build/client.bin -lpthread -lcurses

server:
	gcc server.c -o ./build/server.bin -lpthread -lcurses

clean:
	rm -rf *.o  ./build/server.bin ./build/client.bin

clean_all: clean
	rm -rf *.out *~ 
