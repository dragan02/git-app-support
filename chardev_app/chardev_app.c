/**
 * @file chardev_app.c
 * @brief Character device functionality
 *
 * File represents character device approach to communication with a GPIO. GPIO control
 * is achieved with usage of libgpiod library which, according to its description,
 * "encapsulates the ioctl calls and data structures behind a straightforward API".
 * 
 * Functionality consists of opening proper gpiochip, getting its lines i.e. pins,
 * setting its pins as input or output, after which the state of the input GPIO
 * data lines is being copied to the output GPIO lines.
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
#include <gpiod.h>      // GPIO char. dev. API

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
#define I2C_CTRL_EN_MASK                (0x01)

/** Name of the GPIO consumers */
#define GPIOD_INPUT "gpiod-input"
#define GPIOD_OUTPUT "gpiod-output"

/** Number of GPIO pins which will be used */
unsigned int pin_num = 8;
/** Global pointer on GPIO chip */
struct gpiod_chip *dev_chip;

/**
 * @brief Signal handler function
 *
 * Function catches Ctrl+C signal and closes opened GPIO chip
 *
 */
static void sig_handler(int _)
{
    (void)_;
    
    /** Close GPIO chip */
    gpiod_chip_close(dev_chip); 
    
    printf("\nGPIO chip closed successfully\n");
    
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
		
		printf("I2C data = %d\n", data);
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
            printf("MMS data = %d, ", data);

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
    /** GPIO input and output bulk*/
    struct gpiod_line_bulk input_bulk, output_bulk;
    /** Offsets of input and output lines */
    unsigned int input_lines[] = {0, 1, 2, 3};
    unsigned int output_lines[] = {4, 5, 6, 7};
    /** GPIO pin values */
    int pin_values[4] = {0};
    /* Aux. variable when doing read/write operations */
    int ret;
	/* Pool thread */
    pthread_t i2c_thread, mms_thread;
    
    /** Time spec for event handling*/
    struct timespec ts = { 100, 0 };
	struct gpiod_line_event ev;
	struct gpiod_line *event_line;
    
    unsigned int num_events;
    unsigned int line_offset;
    struct gpiod_line_bulk event_bulk;
    
	pthread_create(&i2c_thread,NULL,&i2c_handler,NULL);
	pthread_create(&mms_thread,NULL,&mms_handler,NULL);
    
    /* Prepare for Ctrl+C signal handling */
    signal(SIGINT, sig_handler);
    
    printf("GPIO app running!\n");

    /************************************************
     * Open GPIO chip
     ************************************************/
    dev_chip = gpiod_chip_open("/dev/gpiochip4");
    //if (dev_chip = NULL){
    /*if (!dev_chip){
        perror("Opening GPIO chip failed!");
        return -1;
    }*/
    printf("Successfully opened chip!\n");
    
    /************************************************
     * Get input bulk
     ************************************************/
    ret = gpiod_chip_get_lines(dev_chip, input_lines, 4, &input_bulk);
    if (ret < 0) {
        printf ("Failed to get input bulk");
        return 0;
    }
    printf("Successfully made input bulk!\n");
    
    /************************************************
     * Get output bulk
     ************************************************/
    ret = gpiod_chip_get_lines(dev_chip, output_lines, 4, &output_bulk);
    if (ret < 0) {
        printf ("Failed to get input bulk");
        return 0;
    }
    printf("Successfully made output bulk!\n");
    
    /************************************************
     * Set lines in input bulk as input, as well as 
     * edge event
     ************************************************/
    ret = gpiod_line_request_bulk_both_edges_events(&input_bulk, GPIOD_INPUT);
    if (ret < 0) {
        printf("Failed to set edge event!");
        return ret;
    }
    printf("Successfully requested input lines and set edge event!\n");
    
    /************************************************
     * Set lines in output bulk as output
     ************************************************/
    ret =  gpiod_line_request_bulk_output(&output_bulk, GPIOD_OUTPUT, pin_values);
    if (ret < 0) {
        printf("Setting GPIO line from input bulk failed!");
        return ret;
    }
    printf("Successfully requested output lines!\n");
    
    /************************************************
    * In case GPIO pin values are already been set
    ************************************************/
    ret = gpiod_line_get_value_bulk(&input_bulk, pin_values);
    if (ret < 0) {
        printf("Failed to get input values\n");
        return ret;
    }
    
    ret = gpiod_line_set_value_bulk(&output_bulk, pin_values);
    if (ret < 0) {
        printf("Failed to set output values\n");
        return ret;
    }
    
    /************************************************
    * Wait for input change and set GPIO output
    ************************************************/
    while (1) 
    {
        ret = gpiod_line_event_wait_bulk(&input_bulk, &ts, &event_bulk);
        
        if (ret < 0) {
            printf("Error occured while waiting for events\n");
            return ret;
        }
        else if (ret == 1) {
            num_events = gpiod_line_bulk_num_lines(&event_bulk);
            
            ret = gpiod_line_get_value_bulk(&input_bulk, pin_values);
            if (ret < 0) {
                printf("Failed to get input values\n");
                return ret;
            }
            
            for (int i = 0; i < num_events; i++){
                event_line = gpiod_line_bulk_get_line(&event_bulk, i);
                
                line_offset = gpiod_line_offset(event_line);
                //printf("Line event offset is %d\n", line_offset);
                
                gpiod_line_event_read(event_line, &ev);
                if (ev.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
                    pin_values[line_offset] = 1;
                }
                else {
                    pin_values[line_offset] = 0;
                }
            }
        
            ret = gpiod_line_set_value_bulk(&output_bulk, pin_values);
            if (ret < 0) {
                printf("Failed to set output values\n");
                return ret;
            }
        }
    }
    
    return 0;    
    
}
