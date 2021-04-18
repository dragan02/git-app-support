CC=arm-linux-gnueabihf-gcc
MCPU=cortex-a9

CFLAGS=-g -mcpu=${MCPU}
LDFLAGS=-lgpiod

OBJS=chardev_app.o

all: chardev_app

chardev_app: ${OBJS}
	${CC} ${CFLAGS} $? -o $@ ${LDFLAGS}

.PHONY: clean

clean:
	rm -f *.o
	rm -f chardev_app
