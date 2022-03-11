
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <gpiod.h>
#include <time.h>
#include "dprintf.h"
#include "project2.h"

#define RED_LED 12
#define GREEN_LED 25
#define BLUE_LED 13

#define DIST_TRIGGER 22
#define DIST_ECHO 23

unsigned int Timeout = 0;

// FUNCTION TO CHANGE LED INTENSITY
int rgb(char * red, char * green, char * blue){

	int loop_i = 0;
	int r,g,b;
	char *ptr;

	struct gpiod_chip *chip;
	struct gpiod_line *lineR, *lineG, *lineB;
	int req, value;

	// EXPORT PWM
	system("echo -n '0' > /sys/class/pwm/pwmchip0/export");
	system("echo -n '1' > /sys/class/pwm/pwmchip0/export");

	// SET PWM0 PERIOD
	int rpd = open("/sys/class/pwm/pwmchip0/pwm0/period", O_WRONLY);
	int pd0 = PWM_PERIOD * 1000000; // period in nanoseconds

	char * str_pd0 = (char*)malloc(sizeof(char));
	sprintf(str_pd0, "%d",(pd0));
	write(rpd, str_pd0, strlen(str_pd0));

	// SET PWM0 DUTY CYCLE
	int rdc = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);

	long int dc0 = (long int)malloc(sizeof(long int));
	dc0 = strtol(red, &ptr, 10); // user entered duty cycle

	char * str_dc0 = (char*)malloc(sizeof(char));
	sprintf(str_dc0, "%d", (dc0));
	write(rdc, str_dc0, strlen(str_dc0));

	// SET PWM1 PERIOD
	int bpd = open("/sys/class/pwm/pwmchip0/pwm1/period", O_WRONLY);
	int pd1 = PWM_PERIOD * 1000000; // period in nanoseconds

	char * str_pd1 = (char*)malloc(sizeof(char));
	sprintf(str_pd1, "%d", (pd1));
	write(bpd, str_pd1, strlen(str_pd1));

	// SET PWM1 DUTY CYCLE
	int bdc = open("/sys/class/pwm/pwmchip0/pwm1/duty_cycle", O_WRONLY);
	long int dc1 = (long int)malloc(sizeof(long int));
	dc1 = strtol(blue, &ptr, 10); // user entered duty cycle

	char * str_dc1 = (char*)malloc(sizeof(char));
	sprintf(str_dc1, "%d", (dc1));
	write(bdc, str_dc1, strlen(str_dc1));

	// ENABLE PWM
	int enable0 = open("/sys/class/pwm/pwmchip0/pwm0/enable", O_WRONLY);
	write(enable0, "1", 1);

	int enable1 = open("/sys/class/pwm/pwmchip0/pwm1/enable", O_WRONLY);
	write(enable1, "1", 1);

	// UNEXPORT PWM CHIP
	system("echo -n '0' > /sys/class/pwm/pwmchip0/unexport");
	system("echo -n '1' > /sys/class/pwm/pwmchip0/unexport");

	chip = gpiod_chip_open("/dev/gpiochip0");

	if (!chip){
		return -1;
	}

	lineR = gpiod_chip_get_line(chip, RED_LED);
	lineG = gpiod_chip_get_line(chip, GREEN_LED);
	lineB = gpiod_chip_get_line(chip, BLUE_LED);

	if (!lineR || !lineG || !lineB){
		gpiod_chip_close(chip);
		return -1;
	}

	req = gpiod_line_request_output(lineR, "gpio_state", GPIOD_LINE_ACTIVE_STATE_LOW);
	req = gpiod_line_request_output(lineG, "gpio_state", GPIOD_LINE_ACTIVE_STATE_LOW);
	req = gpiod_line_request_output(lineB, "gpio_state", GPIOD_LINE_ACTIVE_STATE_LOW);

	DPRINTF("get lines & output %d \n", req);

	if (req){
		gpiod_chip_close(chip);
		return -1;
	}

	do {
		r = loop_i & 1;
		g = (loop_i >> 1) & 1;
		b = (loop_i >> 2) & 1;
		gpiod_line_set_value(lineR, r);
		gpiod_line_set_value(lineG, g);
		gpiod_line_set_value(lineB, b);

		DPRINTF("values = %d, %d, %d, %d \n", loop_i, r, g, b);
		sleep(1);

		loop_i++;

	} while(loop_i <= 40);

	// EXPORT PWM CHIP
	system("echo -n '0' > /sys/class/pwm/pwmchip0/export");
	system("echo -n '1' > /sys/class/pwm/pwmchip0/export");

	// DISABLE PWM
	int disable0 = open("/sys/class/pwm/pwmchip0/pwm0/enable", O_WRONLY);
	write(disable0, "0", 1);

	int disable1 = open("/sys/class/pwm/pwmchip0/pwm1/enable", O_WRONLY);
	write(disable1, "0", 1);

	// UNEXPORT PWM CHIP
	system("echo -n '0' > /sys/class/pwm/pwmchip0/unexport");
	system("echo -n '1' > /sys/class/pwm/pwmchip0/unexport");

	// CLOSE FILES
	close(rpd); close(rdc); 
	close(bpd); close(bdc);
	close(enable0); close(enable1);
	close(disable0); close(disable1);

	gpiod_chip_close(chip);

	return 0;
}

// FROM TESTCC
// function to substract ts2 from ts1. The result is in ts3

void ts_sub(struct timespec *ts1, struct timespec *ts2, struct timespec *ts3)
{
    ts3->tv_sec = ts1->tv_sec - ts2->tv_sec;

    if(ts1->tv_nsec >= ts2->tv_nsec) {
		ts3->tv_nsec = ts1->tv_nsec - ts2->tv_nsec;
	} else
	{
		ts3->tv_sec--;
		ts3->tv_nsec = 1e9 + ts1->tv_nsec - ts2->tv_nsec;
    }
}

// read cycle count in user space.

static inline u_int32_t ccnt_read (void)
{
  u_int32_t cc = 0;
  __asm__ volatile ("mrc p15, 0, %0, c9, c13, 0":"=r" (cc));
  return cc;
}

// FUNCTION TO CALCULATE DISTANCE
int dist(void){

	struct gpiod_chip *chip;
	struct gpiod_line *line_tr, *line_ec;
	int req, value;

	u_int32_t t0, t1, d1, d0;
  	struct timespec c0, c1, c2;

	// EXPORT GPIO
	system("echo 22 > /sys/class/gpio/export");
	system("echo 23 > /sys/class/gpio/export");
	// SET I/O
	int io22 = open("/sys/class/gpio/gpio22/direction", O_WRONLY);
	write(io22, "out", 3);
	int io23 = open("/sys/class/gpio/gpio23/direction", O_WRONLY);
	write(io23, "in", 2);
	// SET TRIGGER ACTIVE HIGH
	int tr_mode = open("/sys/class/gpio/gpio22/active_low", O_WRONLY);
	write(tr_mode, "0", 1);
	sleep(1); //sleep 1 second
	// SET TRIGGER LOW
	write(tr_mode, "1", 1);

	chip = gpiod_chip_open("/dev/gpiochip0");
	if(!chip){
		gpiod_chip_close(chip);
		return -1;
	}

	line_tr = gpiod_chip_get_line(chip, DIST_TRIGGER);
	line_ec = gpiod_chip_get_line(chip, DIST_ECHO);

	if(!line_tr || !line_ec){
		gpiod_chip_close(chip);
		return -1;
	}

	clock_t t;
	t = clock();
	//PING
	req = gpiod_line_request_input(line_ec, "gpio_state");

	while(req == 0){
		t = clock();
	}

	while(req == 1){
		t = clock() - t;
	}

	//CALCULATE DISTANCE
	double distance = ((t/1000) * 34300) / 2;

	// UNEXPORT GPIO
	system("echo 22 > /sys/class/gpio/unexport");
	system("echo 23 > /sys/class/gpio/unexport");

	// CLOSE FILES
	close(io22); close(io23); close(tr_mode);

	gpiod_chip_close(chip);

	return distance;
}



int main(void){
//LOOP TO RUN APPLICATION
	do {

		printf("Enter command: ");
		char buffer[256];
		int result = scanf("%255[^\n]%*c", buffer); //read input

		int numargs = 0;

		// TOKENIZE INPUT
		for(int i = 0; buffer[i]!= '\0'; i++){

			if(buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t'){
				numargs++;
			}
		}

		char **input = (char**)malloc(numargs * sizeof(char*));

		for(int k = 0; k < numargs; k++){
			input[k] = (char*)malloc(numargs * sizeof(char));
		}

		input[0] = strtok(buffer, " ");

		for(int j = 1; j <= numargs; j++){
			input[j] = strtok(NULL, " ");
		}

		//char *ri, *gi, *bi;
		//char *n, *mode;

		//RECOGNIZE COMMAND
		if(strcmp("dist", input[0]) == 0){

			//n = input[1];
			//mode = input[2];
			char * ptr;

			printf("Executing dist...\n\n");
			//printf("%s %d %d\n", input[0], input[1], input[2]);


			long int n = (long int)malloc(sizeof(long int));
			n = strtol(input[1], &ptr, 10); // user entered n value
			for(int l = 0; l < n; l++){
				int d = dist();
				printf("%d\n",d);
			}

		}

		else if(strcmp("rgb", input[0]) == 0){

			//ri = input[1];
			//gi = input[2];
			//bi = input[3];
			printf("Executing rgb...\n\n");
			//printf("%d %d %d\n", input[1], input[2], input[3]);
			rgb(input[1], input[2], input[3]);

		}

		// EXIT COMMAND
		else if(strcmp("exit", input[0]) == 0){

			printf("Exiting...\n");
			exit(0);
		}

		// CATCH INVALID INPUTS
		else{
			printf("Invalid command\n");
		}


	} while(1);

	return 0;
}
