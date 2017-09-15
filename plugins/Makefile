CC=gcc
CFLAGS=-ggdb -O0 -I../include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LDFLAGS=-lvmi -lglib-2.0 -lxenctrl -lxentoollog -lxenlight

OBJECTS=pslist.o pstree.o handle.o vmi_helper.o

SRCS=vmi_helper.c

all: $(OBJECTS)
	ar rcs libxvol.a $^
	rm -f $(OBJECTS)

%.o:%.c 
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -f libxvol.a
	rm -f *.o
