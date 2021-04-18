/**
 * @file chardev_app.c
 * @brief Character device functionality
 *
 * File represents character device approach to communication with a GPIO.
 * 
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
#include <unistd.h>
#include <gpiod.h>      // GPIO char. dev. API
#include <errno.h>      // For error handling
#include <time.h>       // Needed for sleep function
#include <signal.h>     // Needed for signal handling

/**
 * @brief Main
 *
 * Function represents main functionality (GPIO handling)
 *
 */
int main(){
    
    struct gpiod_chip *dev_chip;
    struct gpiod_line_bulk *input_bulk;
    struct gpiod_line_bulk *output_bulk;
    struct gpiod_line *output_line;
    
    unsigned int input_lines[] = {0, 1, 2, 3};
    unsigned int output_lines[] = {4, 5, 6, 7};
    
    int ret, value;
    
    printf("GPIO app running!\n");

	dev_chip = gpiod_chip_open("/dev/gpiochip4");
    if (dev_chip = NULL){
        perror("Opening GPIO chip failed!");
        return -1;
    }
    printf("Successfully opened chip!\n");
    
    output_line = gpiod_chip_get_line(dev_chip, 0);
    printf("Successfully took chip line!\n");;
    
    ret = gpiod_chip_get_lines(dev_chip, input_lines, 4, input_bulk);
        if (ret < 0) {
        perror("Getting GPIO input bulk failed!");
        return ret;
    }
    
    printf("Successfully made input bulk!\n");
    
    ret = gpiod_chip_get_lines(dev_chip, output_lines, 4, output_bulk);
    if (ret < 0) {
        perror("Getting GPIO output bulk failed!");
        return ret;
    }
    
    printf("Successfully made output bulk!\n");
    
    for (int i=0; i<4; i++){
        ret = gpiod_line_request_input(input_bulk->lines[i], "input_test");
        if (ret < 0) {
            perror("Setting GPIO line from input bulk failed!");
            return ret;
        }
    }
    
    printf("Successfully retuested input lines!\n");
    
    for (int i=0; i<4; i++){
        ret = gpiod_line_request_output(output_bulk->lines[i], "output_test", 0);
        if (ret < 0) {
            perror("Setting GPIO line from output bulk failed!");
            return ret;
        }
    }
    
    printf("Successfully retuested output lines!\n");
    
    return 0;    
    
    
    
    
    
    
    
}
