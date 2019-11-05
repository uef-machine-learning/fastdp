CC=g++
GPP=g++

CXXFLAGS +=  -O3 -g $(ARCHFLAGS)  -Wall -I.  -std=c++11 -static
LIBS = -lpthread

CFLAGS = -O1 -g

DEPS = *.cpp *.h
OBJS=argtable3.o options.o
.PHONY:	all clean

PROGS= rknng

all: rknng

# Argtables should support compiling with g++, but there was an error message.
argtable3.o:
	gcc -c $(CFLAGS) contrib/argtable3.c

options.o:
	$(CC) -c $(CXXFLAGS) options.c

rknng: $(DEPS) $(OBJS)
	$(CC) $(CXXFLAGS) $(DISABLEWARN) knng.cpp $(LIBS) $(OBJS) -o rknng 

clean:
	rm -f $(PROGS) *.o

apitest: options.o
	$(GPP) -O3 -c -std=c++11 -o rknng_lib.o rknng_lib.cpp $(LIBS)
	gcc -c options.c
	gcc -c apitest.c
	$(GPP) -o apitest apitest.o rknng_lib.o options.o $(LIBS)

