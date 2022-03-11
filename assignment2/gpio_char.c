#include <stdio.h>    // for printf
#include <fcntl.h>    // for open
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <gpiod.h>
#include "dprintf.h"
//#define DEBUG
#define GREEN_LED 24
#define BLUE_LED 23
#define RED_LED 25

unsigned int Timeout = 0;

int main(void)
{
    int loop_i = 0;
    int Red,Green,Blue;

    struct gpiod_chip *chip;
    struct gpiod_line *lineR, *lineG, *lineB;

    int req, value;

    chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip)
	return -1;

    lineR = gpiod_chip_get_line(chip, RED_LED);
    lineG = gpiod_chip_get_line(chip, GREEN_LED);
    lineB = gpiod_chip_get_line(chip, BLUE_LED);

    if (!lineR || !lineG || !lineB) {
	gpiod_chip_close(chip);
	return -1;
    }

    req = gpiod_line_request_output(lineR, "gpio_state", GPIOD_LINE_ACTIVE_STATE_LOW);
    req = gpiod_line_request_output(lineG, "gpio_state", GPIOD_LINE_ACTIVE_STATE_LOW);
    req = gpiod_line_request_output(lineB, "gpio_state", GPIOD_LINE_ACTIVE_STATE_LOW);
    DPRINTF("get lines & output %d \n", req);

    if (req) {
	gpiod_chip_close(chip);
	return -1;
    }

    do {
	Red = loop_i & 1;
	Green = (loop_i >> 1) & 1;
	Blue = (loop_i >> 2) & 1;
	gpiod_line_set_value(lineR, Red);
	gpiod_line_set_value(lineG, Green);
	gpiod_line_set_value(lineB, Blue);

	DPRINTF("values = %d, %d, %d, %d \n", loop_i, Red, Green, Blue);
	sleep(1);

	loop_i++;

    } while(loop_i <= 40);

	gpiod_chip_close(chip);
	return 0;
}
