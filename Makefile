CC=gcc
CFLAGS=-ggdb -O0 -I./include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LDFLAGS=-lvmi -lglib-2.0 -lxenctrl -lxentoollog -lxenlight

SUBDIRS=

SRC=xvol.c log.c config.c

CFLAGS+= -DCURRENT_LEVEL=LV_WARN
.PHONY : all $(SUBDIRS) clean

all: $(SUBDIRS) xvol

xvol: $(SRC)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(SUBDIRS):
	$(MAKE) -C $@ clean all

clean:
	rm -rf xvol
	for dir in $(SUBDIRS); do\
		$(MAKE) -C $$dir -f Makefile $@; \
	done




