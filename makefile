CC=gcc
CFLAGSSERV=-std=gnu99 -Wall -Wextra -fno-common -lm
CFLAGSCLIENT=-std=c99 -Wall -Wextra -fno-common
CFLAGSOPENGL=-std=c99 -Wall -Wextra -Wsign-compare -fno-common -lX11 -lglut -lGL -lGLU -lm -lSOIL

all:
	$(CC) -o server.exe server.c packets_server.c packets_client.c helpers_common.c server_logic.c $(CFLAGSSERV)
	$(CC) -o client.exe client.c packets_client.c helpers_common.c view.objects.c view.player.c view.tile.c $(CFLAGSCLIENT) $(CFLAGSOPENGL)
	$(CC) -o test_packets.exe test_packets.c packets_server.c packets_client.c helpers_common.c $(CFLAGSSERV) $(CFLAGSCLIENT)
#	$(CC) -o game.exe view.demo.c view.objects.c view.player.c view.tile.c $(CFLAGSOPENGL)

server.exe: server.o
	$(CC) -o server.exe server.o packets_server.c packets_client.c helpers_common.c server_logic.c $(CFLAGSSERV)

cient.exe: client.o
	$(CC) -o client.exe client.o packets_client.c helpers_common.c view.objects.c view.player.c view.tile.c $(CFLAGSCLIENT) $(CFLAGSOPENGL)

test_packets.exe: test_packets.o
	$(CC) -o test_packets.exe test_packets.o packets_server.c packets_client.c helpers_common.c $(CFLAGSSERV) $(CFLAGSCLIENT)

game.exe: view.game.o
	$(CC) -o game.exe view.game.o view.objects.c view.player.c view.tile.c $(CFLAGSOPENGL)

clean:
	rm -f server.exe client.exe game.exe test_packets.exe server.o client.o test_packets.o view.game.o

testserv: all
	./server.exe -p=12343
	make clean

testclient: all
	./client.exe -a=localhost -p=12343
	make clean

testpackets: all
	./test_packets.exe
	make clean

testview: all
	./game.exe
	make clean