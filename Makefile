all:  libepollserv.so mymv
LIB_PATH = 
LIBS = pthread
INCLUDES = ./include/
FLAGS = std=c++11 -g -fPIC -shared
SRCS = ./src/*.cpp
	
libepollserv.so: $(SRCS)
	g++ $^ -o $@ -$(FLAGS) -I$(INCLUDES) -l$(LIBS)
	
.PHONY : clean
clean:
	rm -rf ./bin/* *.o
	
mymv:
	mv libepollserv.so ./lib/