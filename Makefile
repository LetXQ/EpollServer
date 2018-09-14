all:  libepollserv.so client server mymv
LIB_PATH = ./lib
LOCAL_LIBS = epollserv
LIBS = pthread
INCLUDES = ./include/
FLAGS = std=c++11 -g 
OFLAGS = fPIC -shared
SRCS = ./src/*.cpp
	
client: client.cpp
	g++ $^ -g -o $@ -$(FLAGS) -I$(INCLUDES) -L$(LIB_PATH) -l$(LOCAL_LIBS) -l$(LIBS)
	
server: server.cpp
	g++ $^ -g -o $@ -$(FLAGS) -I$(INCLUDES) -L$(LIB_PATH) -l$(LOCAL_LIBS) -l$(LIBS)
	
libepollserv.so: $(SRCS)
	g++ $^ -g -o $@ -$(FLAGS) -$(OFLAGS) -I$(INCLUDES) -l$(LIBS) 
	
.PHONY : clean
clean:
	rm -rf ./bin/* *.o
	rm -rf ./lib/*
	
mymv:
	mv libepollserv.so ./lib/
	mv client ./bin/
	mv server ./bin/