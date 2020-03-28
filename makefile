CC = gcc
PyFLAGS=`python3-config --cflags --ldflags`
CFLAGS = -Wall -std=gnu11 -lwiringPi

all: mainsys.elf

mainsys.elf: mainsys.c mpu9250.c
	$(CC) $(PyFLAGS) $(CFLAGS) $^ -o mainsys.elf

clean:
	rm -f *.o *.elf
