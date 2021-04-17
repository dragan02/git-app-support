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
 * @version [1.0 @ 04/2021] Initial version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>      // For error handling
#include <unistd.h>     // Needed for ftruncate
#include <fcntl.h>      // Defines O_* constants
#include <sys/stat.h>   // Defines mode constants
#include <sys/mman.h>   // Defines mmap flags
#include <time.h>       // Needed for sleep function
#include <signal.h>     // Needed for signal handling

/** Size of buffer */
#define MAX_BUF 64

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
    /* Place where user answer will be stored */
    char  c;

    printf("\nDo you want to unexport GPIO pins? [y/n] ");
    c = getchar();
    
    /* If user chooses, unexport all GPIO pins */
    if (c == 'y' || c == 'Y'){
        int fd, len;
        unsigned int gpio_pin;
        
        fd = open("/sys/class/gpio/unexport", O_WRONLY);
        if (fd < 0) {
            perror("Opening GPIO unexport failed!");
        }
    
        for (int pin=pin_base; pin<(pin_base+pin_num); pin++) {
            len = snprintf(buf, sizeof(buf), "%d", pin);
            write(fd, buf, len);
        }
        
        close(fd);
        printf("GPIOs unexported successfully!\n");
    }
    
    exit(0);

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
    
    /* Prepare for Ctrl+C signal handling */
    signal(SIGINT, sig_handler);
  
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
    /* String representing GPIO edge */
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
     * Change value of the GPIO pins
     ************************************************/
    cnt = 0;

    /* Prepare GPIO pins */
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
        
        cnt++;
    }
    
    printf("GPIOs successfully opened!\n");
    
    /* Read from input pins and write to output ones */
    while (1) 
    {
        for (cnt=0; cnt<(pin_num/2); cnt++){
            ret = read(fds[cnt], &value, 1);
            ret = write(fds[cnt+(pin_num/2)], &value, 1);
            lseek(fds[cnt], 0, SEEK_SET);
        }
        
        /* Sleep */
        usleep(10000);
    }
  
    
    return 0;
}
