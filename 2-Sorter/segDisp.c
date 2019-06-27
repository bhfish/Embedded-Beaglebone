#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "sort.h"
#include "main.h"
#include "a2d.h"
#include "segDisp.h"

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"
#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_61_DIRECTION "/sys/class/gpio/gpio61/direction"
#define GPIO_44_DIRECTION "/sys/class/gpio/gpio44/direction"
#define GPIO_61_VALUE "/sys/class/gpio/gpio61/value"
#define GPIO_44_VALUE "/sys/class/gpio/gpio44/value"
#define DEVICEs_SLOTS "/sys/devices/platform/bone_capemgr/slots"

#define I2C_DEVICE_ADDRESS 0x20

#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15

#define WAIT_SECONDS 0               // number of seconds waiting for each iteration
#define WAIT_NANOSECONDS 5000000   // number of nanoseconds waiting for each iteration

static int initGPIO();
static int turnOnBus();
static int initI2cBus(char* bus, int address);

static unsigned char* getNumPattern(int num);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);

static unsigned char outAPattern[11] = {0xa1, 0x80, 0x31, 0xb0, 0x90, 0xb0, 0xb1, 0x04, 0xb1, 0xb0, 0x10};
static unsigned char outBPattern[11] = {0x86, 0x12, 0x0f, 0x06, 0x8a, 0x8c, 0x8c, 0x14, 0x8e, 0x8e, 0x08};

const struct timespec delayI2C = {1, 0};

static void displayLED()
{
	initGPIO();
	turnOnBus();

	int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

	writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

	long seconds = WAIT_SECONDS;
	long nanoseconds = WAIT_NANOSECONDS;
	struct timespec delay = {seconds, nanoseconds};
		
	int gpioOff = 0;
	int gpioOn = 1;	

	printf(" . . . Start LED Task\n");

	while(progRun){
		//display decimal Number 
		unsigned char* numPattern = getNumPattern(cntr_num);

		writeToFile(GPIO_61_VALUE, &gpioOff, 1);
		writeToFile(GPIO_44_VALUE, &gpioOff, 1);

		writeI2cReg(i2cFileDesc, REG_OUTA, numPattern[0]);
		writeI2cReg(i2cFileDesc, REG_OUTB, numPattern[1]);
		
		writeToFile(GPIO_61_VALUE, &gpioOn, 1);
		nanosleep(&delay, (struct timespec *) NULL);

		writeToFile(GPIO_61_VALUE, &gpioOff, 1);
		writeToFile(GPIO_44_VALUE, &gpioOff, 1);
		writeI2cReg(i2cFileDesc, REG_OUTA, numPattern[2]);
		writeI2cReg(i2cFileDesc, REG_OUTB, numPattern[3]);
		writeToFile(GPIO_44_VALUE, &gpioOn, 1);
		nanosleep(&delay, (struct timespec *) NULL);
		free(numPattern);

	}

	printf(" . . . Stop LED Task\n");

	// Cleanup 
	writeToFile(GPIO_61_VALUE, &gpioOff, 1);
	writeToFile(GPIO_44_VALUE, &gpioOff, 1);

	close(i2cFileDesc);
	
	
	return;

}

static int initGPIO()
{
	int i = 61;
	int j = 44;
	writeToFile( GPIO_EXPORT, &i, 1);
	writeToFile( GPIO_EXPORT, &j, 1);
	writeToFile( GPIO_61_DIRECTION, &"out", 2);
	writeToFile( GPIO_44_DIRECTION, &"out", 2);
	return 0;
}

static int turnOnBus()
{
	pthread_mutex_lock(&write_cape_slot_mutex);
	writeToFile( DEVICEs_SLOTS, &"BB-I2C1", 2);
	pthread_mutex_unlock(&write_cape_slot_mutex);
	return 0;
}

/* set flag=1 to write int, flag=2 to write string
*/
int writeToFile(char* fileDirectory, void* content, int flag)
{
	FILE *file = fopen(fileDirectory, "w");
	if(file == NULL){
		printf("ERROR: Unable to open file (%s) for write\n", fileDirectory);
		exit(-1);
	}

	int charWritten;
	if(flag==1){
		charWritten = fprintf(file, "%d", *(int*) content);
	}else if(flag==2){
		charWritten = fprintf(file, "%s", (char*) content);
	}
	if(charWritten <= 0){
		printf("ERROR WRITING DATA");
		exit(-1);
	}

	fclose(file);
	return 0;
}


static unsigned char* getNumPattern(int num)
{
	unsigned char* pattern = (unsigned char*) malloc(4*sizeof(unsigned char));
	if(num>=100){
		num = 99;	
	}else if(num <0){
		pattern[0] = outAPattern[10];	
		pattern[1] = outBPattern[10];
		pattern[2] = outAPattern[10];	
		pattern[3] = outBPattern[10];
		return pattern;	
	}

	int ones = num%10;
	int tens = num/10;
	pattern[0] = outAPattern[tens];	
	pattern[1] = outBPattern[tens];		
	pattern[2] = outAPattern[ones];
	pattern[3] = outBPattern[ones];		
	return pattern;
}

static int initI2cBus(char* bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0) {
		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(-1);
	}

	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror("Unable to set I2C device to slave address.");
		exit(-1);
	}
	return i2cFileDesc;
}

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		perror("Unable to write i2c register");
		exit(-1);
	}
}


/*
* Initialize and start sorter task
*/
void led_init()
{
	displayLED();
  
	return;
}
