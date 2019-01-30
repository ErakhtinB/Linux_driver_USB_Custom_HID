obj-m += stmled.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
load:
	sudo insmode stmled.ko
unload:
	sudo rmmod stmled.ko
