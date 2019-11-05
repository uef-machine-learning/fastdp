MODULES = ../modules/
OBJECTS = ../modules/
PRGNAME = cbevi
DEPENDS = $(OBJECTS)cb.o          \
          $(OBJECTS)file.o        \
          $(OBJECTS)interfc.o     \
          $(OBJECTS)memctrl.o     \
          $(OBJECTS)random.o      \
          $(OBJECTS)textfile.o      \
          $(OBJECTS)fvec.o      \
          $(OBJECTS)evi.o
	  
OPT     = -std=c99 -O3 -Wall  -I. -I$(MODULES)
LIBS=-lm

all: $(PRGNAME)

$(PRGNAME): $(PRGNAME).o $(DEPENDS) 
	gcc -o $(PRGNAME) $(OPT) $(PRGNAME).o $(DEPENDS) $(LIBS)

$(PRGNAME).o: $(PRGNAME).c
	gcc $(OPT) -c $(PRGNAME).c -o $(PRGNAME).o

$(OBJECTS)%.o: %.c
	gcc $(OPT) -c $< -o $@

$(OBJECTS)%.o: $(MODULES)%.c
	gcc $(OPT) -c $< -o $@

.PHONY : clean
clean: 
	rm $(DEPENDS) $(PRGNAME).o

