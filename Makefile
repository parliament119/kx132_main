INCLUDEDIR =./include
CC=gcc
CFLAGS=-I$(INCLUDEDIR)

EXECUTABLE = kx132

SOURCEDIR=./source
OBJDIR=./build
LIBDIR=./lib
BUILDDIR=./build

LIBS=-li2c -lbcm2835 -lpthread -lm

_DEPS = regs_kx132.h drv_kx132.h i2c_wrapper.h ringbuffer.h trigger.h config_kx132.h macros_kx132.h utility.h spi_wrapper.h tcp.h debug_macros.h
DEPS = $(patsubst %,$(INCLUDEDIR)/%,$(_DEPS))

_OBJ = main.o drv_kx132.o i2c_wrapper.o ringbuffer.o trigger.o config_kx132.o utility.o spi_wrapper.o tcp.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))


$(OBJDIR)/%.o: $(SOURCEDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BUILDDIR)/$(EXECUTABLE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLUDEDIR)/*~ 