CC=arm-linux-gnueabihf-gcc
MCPU=cortex-a9

CFLAGS=-g -mcpu=${MCPU}
LDFLAGS=-lpthread

OBJS=sysfs_app.o

all: sysfs_app

sysfs_app: ${OBJS}
	${CC} ${CFLAGS} $? -o $@ ${LDFLAGS}

.PHONY: clean

clean:
	rm -f *.o
	rm -f sysfs_app
