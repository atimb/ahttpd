# Makefile for ahttpd
#

ahttpd_objects = Logger.o ServerSocket.o Socket.o ahttpd.o

all : ahttpd

CXXFLAGS = -DDEBUG -O0

ahttpd: $(ahttpd_objects)
	g++ -pthread -DDEBUG -O0 -o ahttpd $(ahttpd_objects)

Logger: Logger.cpp
Socket: Socket.cpp
ServerSocket: ServerSocket.cpp
ahttpd: ahttpd.cpp

clean:
	rm -f *.o ahttpd
