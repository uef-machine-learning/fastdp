CC=g++
GPP=g++

CXXFLAGS +=  -O3 -g $(ARCHFLAGS)  -Wall -I.  -std=c++11 -static
LIBS = -lpthread

CFLAGS = -O1 -g

DEPS = *.cpp *.h
OBJS1=options.o knngraph.o dataset.o util.o
OBJS=argtable3.o $(OBJS1)
.PHONY:	all clean

PROGS= rknng

all: rknng

# Argtables should support compiling with g++, but there was an error message.
argtable3.o:
	gcc -c $(CFLAGS) contrib/argtable3.c

options.o:
	$(CC) -c $(CXXFLAGS) options.c
#dataset.o:
#	$(CC) -c $(CXXFLAGS) dataset.c
%.o: %.c
	$(CC) $(CXXFLAGS) -c $(patsubst %.o,%.c,$@) -o $@

rknng: $(DEPS) $(OBJS)
	$(CC) $(CXXFLAGS) $(DISABLEWARN) knng.cpp $(LIBS) $(OBJS) -o rknng 

clean:
	rm -f $(PROGS) *.o

apitest: $(OBJS1)
	$(GPP) -O3 -c -std=c++11 -o rknng_lib.o rknng_lib.cpp $(LIBS)
	$(GPP) -std=c++11 -c options.c
	$(GPP) -std=c++11 -c apitest.c
	$(GPP) -o apitest apitest.o rknng_lib.o $(OBJS1) $(LIBS)

