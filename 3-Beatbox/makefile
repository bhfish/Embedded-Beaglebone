CC=arm-linux-gnueabihf-gcc $(CFLAGS)
PUBDIR=$(HOME)/cmpt433/public
CFLAGS= -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror
LDFLAGS=-lpthread -lm -L$(PUBDIR)/asound_lib_BBB -lasound

OBJECTS=a3-beatbox/beatbox.o a3-beatbox/joystick.o a3-beatbox/publicFunc.o a3-beatbox/audioMixer.o a3-beatbox/audioCtrl.o a3-beatbox/i2c.o a3-beatbox/accelerometer.o a3-beatbox/udpServer.o

.PHONY: all
all: beatbox wavfiles node

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

beatbox: $(OBJECTS)
	$(CC) $^ -o $@ $(CFLAGS) $(LDFLAGS)
	mkdir -p $(PUBDIR)/myApps
	cp -f $@ $(PUBDIR)/myApps/

wavfiles:
	mkdir -p $(PUBDIR)/myApps/beatbox-wav-files
	rm -rf $(PUBDIR)/myApps/beatbox-wav-files/*
	cp -r beatbox-wav/* $(PUBDIR)/myApps/beatbox-wav-files/

node:
	mkdir -p $(PUBDIR)/myApps/beatbox-server-copy/
	chmod a+rwx $(PUBDIR)/myApps/beatbox-server-copy/
	rm -rf $(PUBDIR)/myApps/beatbox-server-copy/*	
	cp -r as3-server/* $(PUBDIR)/myApps/beatbox-server-copy/

.PHONY: clean
clean:
	rm beatbox
	rm -f a3-beatbox/beatbox a3-beatbox/*.o a3-beatbox/*~
