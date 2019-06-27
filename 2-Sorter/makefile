CFLAGS= -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror
CC=arm-linux-gnueabihf-gcc $(CFLAGS)
OBJECTS=main.o udp.o sort.o a2d.o segDisp.o

all: sorter noworky
     
sorter: $(OBJECTS) 
	$(CC) $(OBJECTS) -lpthread -o sorter
	cp sorter $(HOME)/cmpt433/public/myApps/

noworky:noworky.o
	$(CC) noworky.c -o noworky
	cp noworky $(HOME)/cmpt433/public/myApps/

clean:
	rm -f sorter $(OBJECTS)
	rm -f noworky noworky.o

run: sorter
	./sorter
