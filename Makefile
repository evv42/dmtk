# Set env specific things here
CC=gcc
LD=gcc
CFLAGS= -std=gnu99 -funsigned-char -g
LDFLAGS= -lm -lX11

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

HEADERS=$(wildcard *.h)
CFILES=$(wildcard *.c)

OBJS=$(CFILES:.c=.o)

.PHONY: all clean

all: libdmtk.a

libdmtk.a: $(OBJS)
	ar rcs libdmtk.a *.o

clean:
	$(RM) $(OBJS)
	$(RM) libdmtk.a

install: libdmtk.a
	install -d $(PREFIX)/lib/
	install -m 644 libdmtk.a $(PREFIX)/lib/
	install -d $(PREFIX)/include/
	install -m 644 *.h $(PREFIX)/include/
