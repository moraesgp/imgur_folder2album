LINKER=/usr/bin/gcc
CC=/usr/bin/gcc
LIBS=-lm -lcurl -lfreeimage $(OPTLIBS)
PREFIX?=/usr/local

SRCDIR=src
OBJDIR=obj
SOURCES=$(wildcard $(SRCDIR)/*.c)
OBJECTS=$(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(patsubst %.c,%.o,$(SOURCES)))

$(info buildando $(OBJECTS))

DEVCFLAGS=-ggdb -Wall -Wextra -I$(SRCDIR) -rdynamic -DNDEBUG $(OPTFLAGS)
CFLAGS=-O2 -I$(SRCDIR) $(OPTFLAGS)

TARGET=build/imgur
DEVTARGET=runme

$(TARGET): build $(OBJECTS)
	$(LINKER) -o $@ $(OBJECTS) $(LIBS)

$(DEVTARGET): CFLAGS=$(DEVCFLAGS)
$(DEVTARGET): build $(OBJECTS)
	$(LINKER) -o $@ $(OBJECTS) $(LIBS)

$(OBJDIR)/main.o: $(SRCDIR)/main.c $(SRCDIR)/imgur_config.h
	$(CC) -c $(CFLAGS) -o $@ $(SRCDIR)/main.c

$(OBJDIR)/cJSON.o: $(SRCDIR)/cJSON.c $(SRCDIR)/cJSON.h
	$(CC) -c $(CFLAGS) -o $@ $(SRCDIR)/cJSON.c

$(OBJDIR)/curl_helper.o: $(SRCDIR)/curl_helper.c $(SRCDIR)/curl_helper.h
	$(CC) -c $(CFLAGS) -o $@ $(SRCDIR)/curl_helper.c

$(OBJDIR)/idname.o: $(SRCDIR)/idname.c $(SRCDIR)/idname.h
	$(CC) -c $(CFLAGS) -o $@ $(SRCDIR)/idname.c

obj/imgur_helper.o: $(SRCDIR)/imgur_helper.c $(SRCDIR)/imgur_helper.h
	$(CC) -c $(CFLAGS) -o $@ $(SRCDIR)/imgur_helper.c

obj/resize_image.o: $(SRCDIR)/resize_image.c $(SRCDIR)/resize_image.h
	$(CC) -c $(CFLAGS) -o $@ $(SRCDIR)/resize_image.c

build:
	-mkdir -p build
	-mkdir -p $(OBJDIR)

clean:
	-rm -rf build $(OBJDIR)
	-rm runme

