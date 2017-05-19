server: SockLib.o server.o HttpServerClass.o
	g++ -Wall -std=c++11 -o server server.o SockLib.o HttpServerClass.o

server.o: HttpServerClass.h server.cpp
	g++ -std=c++11 -Wall -c server.cpp

HttpServerClass.o: SockLib.h HttpServerClass.h HttpServerClass.cpp CGIComponentClass.h CGIComponentClass.cpp
	g++ -std=c++11 -Wall -c HttpServerClass.cpp

SockLib.o: SockLib.h SockLib.cpp
	g++ -std=c++11 -Wall -c SockLib.cpp

all: server;

clean:
	rm *.o server
