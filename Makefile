IDIR =/include
CC=gcc
CFLAGS=-I $(IDIR)

ODIR=./src/obj
CPPDIR=./src

_DEPS = MOS6502.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o MOS6502.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(CPPDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

NES: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

debug: CCFLAGS += -DDEBUG -g
debug: NES