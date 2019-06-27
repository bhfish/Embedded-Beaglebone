/* 
This program will display welcome message, check whether joystick is up, 
blink LED 3 times if joystick is up or blink LED 1 time if not,
and exit if joystick is pressed up for consecutive 10 iterations.
*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define JOY_STICK_UP_VALUE_FILE "/sys/class/gpio/gpio26/value"
#define LED_BRIGHTNESS_FILE "/sys/class/leds/beaglebone:green:usr0/brightness"
#define LED_ON_BRIGHTNESS 1          // the brightness value to turn on LED
#define LED_OFF_BRIGHTNESS 0         // the brightness value to turn off LED
#define NUM_BLINKS_UP 3              // number of times that LED blinks if joystick is pressed up
#define NUM_BLINKS_OFF 1             // number of times that LED blinks if joystick is not pressed up
#define BLINK_SECONDS 0              // number of seconds that LED blinks
#define BLINK_NANOSECONDS 100000000  // number of nanoseconds that LED blinks
#define WAIT_SECONDS 0               // number of seconds waiting for each iteration
#define WAIT_NANOSECONDS 600000000   // number of nanoseconds waiting for each iteration

//check if joystick is pressed up
_Bool checkIfPressedUp()
{
	FILE *file = fopen(JOY_STICK_UP_VALUE_FILE, "r");
	if(file == NULL){
		printf("ERROR: Unable to open file (%s) for read\n", JOY_STICK_UP_VALUE_FILE);
		exit(-1);
	}
	
	const int max_length = 2;
	char buff[max_length];
	fgets(buff, max_length, file);
	
	fclose(file);
	
	if(strcmp("0", buff)==0){
		return true;
	}

	return false;
}

// turn on/off LED
int changeLEDBrightness(int LEDStatus)
{
	FILE *file = fopen(LED_BRIGHTNESS_FILE, "w");
	if(file == NULL){
		printf("ERROR: Unable to open file (%s) for write\n", LED_BRIGHTNESS_FILE);
		exit(-1);
	}

	int charWritten = fprintf(file, "%d",LEDStatus);
	if(charWritten <= 0){
		printf("ERROR WRITING DATA");
		exit(-1);
	}

	fclose(file);
	return 0;
}

// flash LED for num times
int flashLED(int num)
{
	long seconds = BLINK_SECONDS;
	long nanoseconds = BLINK_NANOSECONDS;
	struct timespec delay = {seconds, nanoseconds};
	
	for(int i = 0; i<num; i++){
		changeLEDBrightness(LED_ON_BRIGHTNESS);
		nanosleep(&delay, (struct timespec *) NULL);	
		changeLEDBrightness(LED_OFF_BRIGHTNESS);
		nanosleep(&delay, (struct timespec *) NULL);
	}
	return 0;
}

int main(int argc, char* args[])
{
	printf("Hello embedded world, from Baihui Zhang!\n");

	int counter = 0;

	while(1){
		_Bool isPressed = checkIfPressedUp();
		if(isPressed){
			counter++;
			if(counter>=10){
				break;
			}
			printf("Flashing %d time(s): Joystick = 1 & counter = %d\n",NUM_BLINKS_UP, counter);
			flashLED(NUM_BLINKS_UP);	
		}else{
			counter=0;
			printf("Flashing %d time(s): Joystick = 0 & counter = %d\n",NUM_BLINKS_OFF, counter);
			flashLED(NUM_BLINKS_OFF);		
		}
		long seconds = WAIT_SECONDS;
		long nanoseconds = WAIT_NANOSECONDS;
		struct timespec delay = {seconds, nanoseconds};
		nanosleep(&delay, (struct timespec *) NULL);
	}

	printf("Thank you for the blinks!\n");
	return 0;
}
