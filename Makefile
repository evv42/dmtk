# Set env specific things here
CC=gcc
LD=gcc
#Xlib hates optimization
XFLAGS= -std=gnu99 -funsigned-char -g
CFLAGS= $(XFLAGS) -O3
LDFLAGS= -lm -lX11

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

objects = dmtk.o dmtksio.o

.PHONY: all clean

all: libdmtk.a

$(objects): %.o: %.c

dmtkgui.o:
	$(CC) $(XFLAGS) -c -o dmtkgui.o dmtkgui.c

libdmtk.a: dmtkgui.o $(objects)
	ar rcs libdmtk.a *.o

clean:
	$(RM) *.o
	$(RM) libdmtk.a

install: libdmtk.a
	install -d $(PREFIX)/lib/
	install -m 644 libdmtk.a $(PREFIX)/lib/
	install -d $(PREFIX)/include/
	install -m 644 *.h $(PREFIX)/include/
