PLEASE READ THIS FILE IN ITS ENTIRETY AS IT CONTAINS INFORMATION
RELEVANT TO LICENSING AND USAGE!

File Last Modified: Friday, 08 September 2006

Greetings!

David Wolpoff (david AT sparkfun DOT com) is the guy
who put this all together. Address any questions to him
and they may or may not be answered.

What is this?
The code which contains this README is a collection of C and assembly modules
written for the ARM based Philips LPC2138, specifically the Olimex LPC-P2138 
board. 

No, really! What is it?
This is an I2C Example rig that works with several I2C based modules produced
by Spark Fun Electronics (www.sparkfun.com). It comes with a bunch of
supporting routines, such as an interrupt driven serial I/O interface.

Can I use it?
The short answer is "SURE! Please do!"
The long answer is, yes, but be aware of licensing. Two of the included
files are originally sourced from NewLib which is supported by RedHat,
is GPL, all that good stuff. That means that you've got to be aware 
of the GPL and how it affects you. Refer to http://sources.redhat.com/newlib/
The remainder of the files are written by me (except where specifically 
noted), or modified by me beyond all recognition and inherited from
fellow Spark Fun Employees. That means:
1. You can do what you like with the code I wrote, so long as you keep my 
   name attached.
2. Don't be selfish! If you're writing code based on this, share around! 
   Post to the Spark Fun forums and help others. 


How do I use this?
That's a good question! I did all my coding and development from a Linux 
environment. I used GNUARM (www.gnuarm.com) for cross compilation,
lpc21isp (http://guest.engelschall.com/~martin/lpc21xx/isp/) for programming,
and openOCD (http://openocd.berlios.de/web/) for debugging.
Chances are you're a Windows user. I suggest you start using Linux.
But since you're probably not going to take that advice, I recommend
you read the Arm Cross development tutorial by Jim Lynch found on the
Spark Fun tutorial page (http://www.sparkfun.com/commerce/hdr.php?p=tutorials)
and also read around the Spark Fun Support Forums 
(http://www.sparkfun.com/cgi-bin/phpbb/)


What's included?
COPYING - License file from NewLib.          
COPYING.LIB - License file from NewLib.
COPYING.LIBGLOSS - License from NewLib.
COPYING.NEWLIB - License from Newlib.
LPC214x.h - LPC214x header file defining registers. From Philips.
Makefile - It's a makefile. If you don't know how to use make, I'd learn.
README - This file.
circbuf.c - Circular buffer implementation. By David Wolpoff.
circbuf.h - Header for circbuf.c.
crt.S - startup code from Olimex, modded by David Wolpoff.
iic.c - i2c implementation by David Wolpoff.
iic.h - header file for iic.h.
intcomm.c - interrupt driven IO module for LPC by David Wolpoff.
intcomm.h - header for intcomm.c
lpc2138.cmd - linker script. From Olimex, modded by David Wolpoff.
main.c - i2c example drive routine. By David Wolpoff
printmacros.h - Silly Macros by David Wolpoff.
swi.h - header file gotten from NewLib. Please refer to licenses.
syscalls.c - Stubs file from NewLib. Modded by David Wolpoff. Refer to license.
system.c - lpc2138 support routines. By David Wolpoff and Owen Osborn.
system.h - header for system.h
