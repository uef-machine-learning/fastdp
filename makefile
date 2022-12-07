MODULES = modules/
OBJECTS = modules/
CC=gcc
GPP=g++

# CC_OPT1 = -c -O3  -std=c99 -fopenmp -fpermissive  -Wall  -I. -Imodules/
CC_OPT1 = -O3  -std=c++11 -fopenmp -fpermissive  -Wall  -I. -Imodules/


CC_OPT2 = -lm -static

BINS=dencl
OBJS=modules/file.o main.o dencl.o rknng/util.o rknng/dataset.o rknng/knngraph.o modules/evi.o rknng/rknng_lib.o rknng/options.o modules/cb.o modules/interfc.o  modules/stack.o modules/sort.o modules/random.o modules/textfile.o modules/fvec.o modules/argtable3.o

OPT     = -O3 $(DEBUG) -std=c99  -Wall  -I. -I$(MODULES)  

.PHONY : all clean
all: dencl


dencl.o: dencl/dencl.c
	$(GPP) -c $(CC_OPT1) dencl/dencl.c -lm -static
	
main.o: dencl/main.c
	$(GPP) -c $(CC_OPT1)  dencl/main.c $(CC_OPT2)
	
# dataset.o: dencl/dataset.c
#	$(CC) $(CC_OPT1)  dencl/dataset.c $(CC_OPT2)
	

dencl: $(OBJS)
	$(GPP) -o denc $(CC_OPT1) $(OBJS) -lm

%.o: %.c
	$(CC) $(OPT) -c $(patsubst %.o,%.c,$@) -o $@

clean:
	rm  denc cbevi/cbevi rknng/apitest rknng/*.a */*.o *.o

