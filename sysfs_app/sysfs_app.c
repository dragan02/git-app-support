/**
 * @file sysfs_app.c
 * @brief sysfs functionality
 *
 * File represents (deprecated) sysfs approach to communication with a GPIO.
 * Functionality consists of exporting the proper GPIO pins, setting their 
 * direction, interrupt related properties, after which the state of the input 
 * GPIO data pins is being copied to the output GPIO pins
 * 
 * Pins 0-3 are input, whilst 4-7 are output
 *
 * @date 2021
 * @author Dragan Bozinovic (bozinovicdragan96@gmail.com)
 *
 * @version [1.0 @ 05/2021] Initial version
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>      // For error handling
#include <unistd.h>     // Needed for ftruncate
#include <fcntl.h>      // Defines O_* constants
#include <sys/stat.h>   // Defines mode constants
#include <sys/mman.h>   // Defines mmap flags
#include <time.h>       // Needed for sleep function
#include <signal.h>     // Needed for signal handling
#include <pthread.h>    // Needed for multi-threading

/** Includes needed for periodicity */
#include <sys/time.h>
#include <sys/timerfd.h>
/** Includes needed for polling */
#include <poll.h>
#include <sys/epoll.h>
/** Includes needed for proper I2C functionality */
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

/** I2C parameters - address, registers and mask */
#define CUSTOM_I2C_SENS_ADDR (27)
#define I2C_CTRL_OFFSET                 (0x0)
#define I2C_DATA_OFFSET                 (0x1)
#define I2C_CTRL_EN_SHIFT               (1)
#define I2C_CTRL_EN_MASK                (0x01)

/** Size of buffer */
#define MAX_BUF 64

/** Macro for GPIO polling */
#define POLLGPIO (POLLPRI | POLLERR)

/** Base GPIO number, corresponding to first pin of PL061 GPIO controller*/
unsigned int pin_base = 2027;
/** Number of GPIO pins which will be used */
unsigned int pin_num = 8;
/** Buffer which holds strings needed for different operations */
char buf[MAX_BUF];

/**
 * @brief Signal handler function
 *
 * Function catches Ctrl+C signal and lets user choose should
 * GPIO pins be unexported
 *
 */
static void sig_handler(int _)
{
    (void)_;
    int fd, len;
    unsigned int gpio_pin;

    printf("\nUnexporting GPIO pins..\n");

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        perror("Opening GPIO unexport failed");
    }

    for (int pin=pin_base; pin<(pin_base+pin_num); pin++) {
        len = snprintf(buf, sizeof(buf), "%d", pin);
        write(fd, buf, len);
    }

    close(fd);
    printf("GPIOs unexported successfully\n");
    
    exit(0);

}

/** Structure which containts timer file descriptor and number of missed events */
struct periodic_info { 
	int timer_fd;
	unsigned long long wakeups_missed;
};

/**
 * @brief Enable periodical timer
 *
 * Function which makes periodical timer
 *
 */
static int make_periodic(unsigned int period, struct periodic_info *info) {
    /* Timer file descriptor */
	int fd;
    /* Structure which specifies when timer expires */
	struct itimerspec itval;
    /* Auxiliary variables */
	unsigned int ns;
 	unsigned int s;
	int ret;
    
    /* Create timer object */
	fd = timerfd_create(CLOCK_MONOTONIC, 0);
    
    /* Reset info fields */
	info->wakeups_missed = 0;
	info->timer_fd = fd;
    
    /* Calculate periodicity in seconds and nanoseconds */
	s = period / 1000000;
 	ns = (period - (s * 1000000)) * 1000;
    
    /* Define timer values */
    itval.it_interval.tv_sec = s;   // period in seconds
	itval.it_interval.tv_nsec = ns; // period in ns
	itval.it_value.tv_sec = s;      // initial delay in seconds
	itval.it_value.tv_nsec = ns;    // initial delay in ns
    
    /* Set timer expiration */
	ret = timerfd_settime(fd, 0, &itval, NULL);
    
	return ret;
}

/**
 * @brief Wait period
 *
 * Function which waits for timer event
 *
 */
static void wait_period(struct periodic_info *info) {
	unsigned long long missed;
	int ret;    
    
 	/* Wait for the next timer event. If we have missed any thenumber is written to "missed" */
	ret = read(info->timer_fd, &missed, sizeof(missed));
	info->wakeups_missed += missed;
}

/**
 * @brief I2C thread
 *
 * Function which represents I2C thread. It consists of opening, enabling and reading data
 * from I2C slave, after which result is constantly printed to display.
 *
 */
void *i2c_handler(){
    /* Periodicity structure */
    struct periodic_info info;
	/* I2C file descriptor */
    int fd;
	/* Buffer used for communication */
    char buffer[4];
	/* Aux. variables for storing data */
	uint8_t data;
    
    printf("I2C thread started\n");
	
	/* Open I2C and set slave address */
	fd = open("/dev/i2c-2",O_RDWR);
	if (fd < 0){
		printf("Can't open i2c-2\n");
	}
	if (ioctl(fd, I2C_SLAVE, CUSTOM_I2C_SENS_ADDR) < 0) { 
		printf("Can't set I2C slave address\n");
    }
	
	/* Enable I2C slave */
    buffer[0] = I2C_CTRL_OFFSET;
    buffer[1] = I2C_CTRL_EN_MASK;
    write(fd, buffer, 2) ;
	
	/* Enable timer */
	make_periodic(1000000, &info);

    while (1) {
	    /* Wait for timer event */
		wait_period(&info);
		
		/* Read from data register */
		buffer[0] = I2C_DATA_OFFSET;
		write(fd,buffer,1);
		read(fd,buffer,1);
		
		/* Store in aux. var */
		data = (int8_t)buffer[0];
		
		printf("I2C_data = %d\n", data);
    }
	
	close(fd);
}

/**
 * @brief MM sensor thread
 *
 * Function which represents memory-mapped sensor thread. It consists of enabling sensor,
 * its' interrupts after which data is being read and printed on display.
 *
 */
void *mms_handler(){
    /* MM sensor sysfs file descriptor */
    int sysfs_fd;
    /* Aux. variable when doing read/write operations */
    int ret;
	/* Buffer needed for read and write */
    char buffer[10] = {0};
	/* Aux. variables for storing data */
	uint8_t data;
	/* Pool struct */
	struct pollfd pfd;
    
    printf("MMS thread started!\n");
    
    /* Enable MM sensor */
	sysfs_fd = open("/sys/class/custom_mms/custom_mms0/enable", O_RDWR);
	if (sysfs_fd < 0){
		printf("Can't open enable\n");
	}
	buffer[0] = 1;
	if (write(sysfs_fd,buffer,1) != 1){
		printf("Can't write enable\n");
	}
	close(sysfs_fd);
	//printf("MMS enabled\n");
	
	/* Enable MM sensor interrupt*/
	sysfs_fd = open("/sys/class/custom_mms/custom_mms0/enable_interrupt", O_RDWR);
	buffer[0] = 1;
	if (write(sysfs_fd,buffer,1) != 1){
		printf("Can't write interrupt enable\n");
	}
	close(sysfs_fd);
	//printf("MMS interrupt enabled\n");
	

	/* Prepare for reading */
	sysfs_fd = open("/sys/class/custom_mms/custom_mms0/data", O_RDONLY);
	if (sysfs_fd < 0){
        printf("Can't open data\n");
	}

    ret = read(sysfs_fd, buffer, sizeof(buffer));
    lseek(sysfs_fd,0,SEEK_SET);

	/* Prepare for pooling */
	pfd.fd = sysfs_fd;
	pfd.events = POLLPRI|POLLERR;
    
    while(1) {
        /* Pool */
        ret = poll(&pfd, 1, -1);

        if (ret > 0) {
            /* Read from data register */
            ret = read(sysfs_fd, buffer, sizeof(buffer));

            /* Convert to integer and print */
            data = atoi(buffer);
            printf("MMS_data = %d, ", data);

            /* Reset file descriptor pointer */
            lseek(pfd.fd,0,SEEK_SET);
        }
    }
    
}

/**
 * @brief Main
 *
 * Function represents main functionality (GPIO handling)
 *
 */
int main(){
    /* File descriptor */
	int fd;
    /* Array of file descriptors */
    int fds[pin_num];
    /* Length of string written in buffer*/
    int len;
    /* Variable which counts processed pins */
    int cnt;
    /* GPIO pin being processed */
    unsigned int gpio_pin;
    /* Value of GPIO pin */
    char value;
    /* Aux. variable when doing read/write operations */
    int ret;
	/* Pool thread */
    pthread_t i2c_thread, mms_thread;
    /* Poll structure needed for GPIO pin interrupts*/
    struct pollfd pfds[4];
    
    /* Prepare for Ctrl+C signal handling */
    signal(SIGINT, sig_handler);
    
    /* Create I2C and MMS thread */
	pthread_create(&i2c_thread,NULL,&i2c_handler,NULL);
	pthread_create(&mms_thread,NULL,&mms_handler,NULL);
  
    printf("GPIO app running!\n");
  
    /************************************************
     * Export GPIO pins
     ************************************************/
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		perror("Opening GPIO export failed!");
		return fd;
	}
 
    for (int pin=pin_base; pin<(pin_base+pin_num); pin++) {
        len = snprintf(buf, sizeof(buf), "%d", pin);
        write(fd, buf, len);
    }
    
	close(fd);
    printf("GPIOs exported successfully!\n");
  
    /************************************************
     * Update the direction of the GPIO pins
     ************************************************/
    cnt = 0;
    for (int pin=pin_base; pin<(pin_base+pin_num); pin++) {
        len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", pin);
        
        fd = open(buf, O_WRONLY);
        if (fd < 0) {
            perror("Opening GPIO direction failed!");
            return fd;
        }

        if (cnt < (pin_num/2))
            write(fd, "in", 3);
        else
            write(fd, "out", 4);
        
        close(fd);
        cnt++;
    }
  
    printf("GPIOs direction set as output successfully!\n");
  
    /************************************************
     * Update the IRQ edge of the GPIO pins
     ************************************************/
    /* String representing GPIO edge - both rising and falling */
    const char* edge = "both";
    
    for (int pin=pin_base; pin<(pin_base+pin_num/2); pin++) {
        len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/edge", pin);
        
        fd = open(buf, O_WRONLY);
        if (fd < 0) {
            perror("Opening GPIO edge failed!");
            return fd;
        }

        write(fd, edge, strlen(edge) + 1);
        
        close(fd);
    }
  
    printf("GPIOs IRQ edge set\n");

    /************************************************
     * Prepare GPIO pins
     ************************************************/
    cnt = 0;
    
    for (int pin=pin_base; pin<(pin_base+pin_num); pin++) {
        len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", pin);

        if (cnt < (pin_num/2)) {
            fds[cnt] = open(buf, O_RDONLY);
            if (fds[cnt] < 0) {
                perror("Opening input GPIO value failed!");
                return fds[cnt];
            }
        }
        else {
            fds[cnt] = open(buf, O_WRONLY);
            if (fds[cnt] < 0) {
                perror("Opening output GPIO value failed!");
                return fds[cnt];
            }
        }
        
        /* Increment pin offset */
        cnt++;
    }
    
    printf("GPIOs successfully opened!\n");
    
    /************************************************
     * Bound GPIO file descriptor to poll structure and set event type
     ************************************************/
    pfds[0].fd = fds[0];
    pfds[1].fd = fds[1];
    pfds[2].fd = fds[2];
    pfds[3].fd = fds[3];
    pfds[0].events = POLLGPIO;
    pfds[1].events = POLLGPIO;
    pfds[2].events = POLLGPIO;
    pfds[3].events = POLLGPIO;
  
    /* Discard first IRQ */
    poll(pfds, 4, -1);
    
    /************************************************
     * Wait for input change and set GPIO output
     ************************************************/
    while (1){
        ret = poll(pfds, 4, -1);
        
        if (ret > 0) {
            if ((pfds[0].revents & POLLGPIO)){
                lseek(fds[0], 0, SEEK_SET);
                ret = read(fds[0], &value, 1);
                ret = write(fds[4], &value, 1);
            }
            
            if ((pfds[1].revents & POLLGPIO)){
                lseek(fds[1], 0, SEEK_SET);
                ret = read(fds[1], &value, 1);
                ret = write(fds[5], &value, 1);
            }
            
            if ((pfds[2].revents & POLLGPIO)){
                lseek(fds[2], 0, SEEK_SET);
                ret = read(fds[2], &value, 1);
                ret = write(fds[6], &value, 1);
            }
            
            if ((pfds[3].revents & POLLGPIO)){
                lseek(fds[3], 0, SEEK_SET);
                ret = read(fds[3], &value, 1);
                ret = write(fds[7], &value, 1);
            }
        }
    }
  
    
    return 0;
}
