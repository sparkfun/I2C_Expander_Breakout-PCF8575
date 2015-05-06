# Makefile, modified from Olimex.
NAME   = LPC_Tests
CC      = arm-elf-gcc
LD      = arm-elf-ld -v
AR      = arm-elf-ar
AS      = arm-elf-as
CP      = arm-elf-objcopy
OD	= arm-elf-objdump
CFLAGS  = -I./ -c -march=armv4t -g -Os
AFLAGS  = -ahls -mapcs-32 -o crt.o
# Needs modification to work. :-)
LDIRS   = -L /home/david/LPC_Workspace/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2/ -L /home/david/LPC_Workspace/gnuarm-4.0.2/arm-elf/lib/
LFLAGS  = -Map main.map -Tlpc2138.cmd $(LDIRS)
LINC    = -lc -lgcc
CPFLAGS = -O ihex
ODFLAGS	= -x --syms
# OBJS = crt.o syscalls.o main.o i2c.o circbuf.o intcomm.o system.o
OBJS = crt.o  main.o circbuf.o intcomm.o system.o syscalls.o iic.o

all: main

clean:
	rm -rf crt.lst main.lst main.hex main.map main.dmp
	rm -rf $(OBJS)

main: main.out
	@ echo "...copying"
	$(CP) $(CPFLAGS) main.out main.hex
	$(OD) $(ODFLAGS) main.out > main.dmp

main.out: crt.o  lpc2138.cmd $(OBJS) syscalls.o
	@ echo "..linking"
	$(LD) $(LFLAGS) -o main.out $(OBJS) $(LINC) 

crt.o: crt.S
	@ echo ".assembling crt.S"
	$(AS) $(AFLAGS) crt.S > crt.lst

main.o: main.c
	@ echo ".compiling main"
	$(CC) $(CFLAGS) main.c

iic.o: iic.h iic.c
	@ echo ".compiling iic"
	$(CC) $(CFLAGS) iic.c

i2c.o: i2c.h i2c.c
	@ echo ".compiling i2c"
	$(CC) $(CFLAGS) i2c.c

circbuf.o: circbuf.h circbuf.c
	@ echo ".compiling circbuf"
	$(CC) $(CFLAGS) circbuf.c

system.o: system.h system.c
	@ echo ".compiling system"
	$(CC) $(CFLAGS) system.c

intcomm.o: intcomm.h intcomm.c
	@ echo "Compiling Intcomm"
	$(CC) $(CFLAGS) intcomm.c

program: main
	lpc21isp -control main.hex /dev/ttyS0 115200 14745

syscalls.o: syscalls.c
	$(CC) $(CFLAGS) syscalls.c
