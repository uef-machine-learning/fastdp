MODULES = modules/
OBJECTS = modules/
CC=gcc
GPP=g++

CCOPT1=-O3  -std=c++11 -fopenmp -fpermissive  -Wall  -I. -Imodules/ 
CC_OPT2=-lm -static

BINS=dencl
OBJS=modules/file.o dencl/main.obj cbevi/evi.o modules/cb.o modules/interfc.o  modules/stack.o modules/sort.o modules/random.o modules/textfile.o modules/fvec.o modules/argtable3.o

OPT     = -O3 $(DEBUG) -std=c99  -Wall  -I. -I$(MODULES)  

.PHONY : all clean
all: dencl


dencl: $(OBJS)
	$(GPP) -o denc $(CCOPT1) $(OBJS) -lm
	
%.o: %.c
	$(CC) $(OPT) -c $(patsubst %.o,%.c,$@) -o $@

%.obj: %.cpp
	$(GPP) $(CCOPT1) -c $(patsubst %.obj,%.cpp,$@) -o $@ $(CC_OPT2) 

clean:
	rm  denc cbevi/cbevi rknng/apitest rknng/*.a */*.o *.o */*obj

