CC = gcc
PyFLAGS=`python3-config --cflags --ldflags`
CFLAGS = -Wall -std=gnu11

all: mainsys.elf

mainsys.elf: mainsys.c
	$(CC) $(PyFLAGS) $(CFLAGS) $^ -o mainsys.elf

clean:
	rm -f *.o *.elf
