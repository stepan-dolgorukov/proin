obj-m := proin.o

all: proin proinya

proin:
	make --directory="/lib/modules/$(shell uname --kernel-release)/build" M="$(shell pwd)"

proinya: proinya.o
	gcc proinya.o -o proinya

proinya.o: proinya.c
	gcc -c proinya.c

clean:
	rm proinya.o proinya
	make --directory="/lib/modules/$(shell uname --kernel-release)/build" M="$(shell pwd)" clean

