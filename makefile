MODULES = modules/
OBJECTS = modules/
CC=gcc
GPP=g++


BINS=dencl
OBJS=modules/file.o  dencl.o  modules/evi.o rknng/rknng_lib.o rknng/options.o modules/cb.o modules/interfc.o  modules/stack.o modules/sort.o modules/random.o modules/textfile.o modules/fvec.o modules/argtable3.o

OPT     = -O3 $(DEBUG) -std=c99  -Wall  -I. -I$(MODULES)  


.PHONY : all clean
all: dencl



dencl.o: dencl/dencl.c
	$(CC) -c -O3  -std=c99 -fopenmp -fpermissive  -Wall  -I. -Imodules/ -Idencl/contrib/libstrcmp/src/  dencl/dencl.c -lm -static

dencl: $(OBJS)
	$(GPP) -o denc -O3  -std=c99 -fopenmp -fpermissive  -Wall  -I. -Imodules/ $(OBJS) -lm

%.o: %.c
	$(CC) $(OPT) -c $(patsubst %.o,%.c,$@) -o $@

clean:
	rm  denc cbevi/cbevi rknng/apitest rknng/*.a */*.o *.o

