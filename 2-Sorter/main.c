/*
*  CMPT 433 Assignment 2
*  Group: En Joy
*  Name: Teresa Dun
*  SFU ID: 301302027
*  Name: Baihui Zhang
*  SFU ID: 301316494
*  Description: a C program which runs on the target to sort arrays of integers and listen to a UDP socket.
*  The program will use Zen cape's potentiometer to allow the user to select the size of array to sort and 
*  dispaly the number of arrays sorted per second on the 2 character 14-segment display.
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "udp.h"
#include "sort.h"
#include "a2d.h"
#include "segDisp.h"
#include "main.h"

bool progRun = true;

pthread_mutex_t arrayLen_mutex;
pthread_mutex_t arraySort_mutex;
pthread_mutex_t arrayRead_mutex;
pthread_mutex_t write_cape_slot_mutex;

/*
* Stop all tasks
*/
void programExit()
{
	progRun = false;
}


int main(int argc, char *argv[])
{
	int rt;
	pthread_t udp_thread;
	pthread_t sorter_thread;
	pthread_t a2d_thread;
	pthread_t i2c_thread;


	//Start a2d thread
	rt = pthread_create(&a2d_thread, NULL,  (void *)&a2d_init, NULL);
    if( rt )
	{
		printf("Thread creation failed: %d\n", rt);
		return -1;	
	}

	//Start sorter  thread
	rt = pthread_create(&sorter_thread, NULL,  (void *)&sorter_init, NULL);
    if( rt )
	{
		printf("Thread creation failed: %d\n", rt);
		return -1;	
	}

	//Start i2c thread
	rt = pthread_create(&i2c_thread, NULL,  (void *)&led_init, NULL);
    if( rt )
	{
		printf("Thread creation failed: %d\n", rt);
		return -1;	
	}

	//Start UDP thread
	rt = pthread_create(&udp_thread, NULL,  (void *)&udp_init, NULL);
    if( rt )
	{
		printf("Thread creation failed: %d\n", rt);
		return -1;	
	}

	pthread_join( sorter_thread, NULL);
	pthread_join( udp_thread, NULL);
	pthread_join( a2d_thread, NULL);
	pthread_join( i2c_thread, NULL);

	return 0;
}
