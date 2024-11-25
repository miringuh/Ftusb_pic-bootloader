FILE=ftusb
# FILE=usb_1
URL=https://github.com/mainahjeff/ftdi2pic16-18_programmer.git
AC=gh repo clone mainahjeff/ftdi2pic16-18_programmer

make:
#	make clean
#	FTDI----
	clear
	gcc -o ${FILE}.o ${FILE}.c /usr/lib/x86_64-linux-gnu/libftdi1.so	
#	USB-----
#	gcc -o ${FILE}.o ${FILE}.c /usr/lib/x86_64-linux-gnu/libusb-1.0.so 
	./${FILE}.o
clean:	
	rm -f ${FILE}
	rm -f *.o *./${FILE}
	clear
