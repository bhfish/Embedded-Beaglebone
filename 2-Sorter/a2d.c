#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "sort.h"
#include "a2d.h"
#include "segDisp.h"

/*
* Define constant
*/
#define A2D_FILE_VOLTAGE0 "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define CAPE_LOADER "/sys/devices/platform/bone_capemgr/slots"

const struct timespec delay1s = {1, 0};
int cntr_num = 0;
int gen_array_len = 100;

int a2d_set_bus(void)
{
	pthread_mutex_lock(&write_cape_slot_mutex);
	writeToFile( CAPE_LOADER, &"BB-ADC", 2);
	pthread_mutex_unlock(&write_cape_slot_mutex);
	return 0;
}

int a2d_read_0(void)
{
	FILE *f = NULL;
	int val = 0;
	int res;

	//Open a2d voltage file
//	f = fopen(a2d_file_vltg0, "r");
	f = fopen(A2D_FILE_VOLTAGE0, "r");

	if (f == NULL) 
	{
		printf("...ERROR: open voltage input file.\n");

		return -1;
	}

	//Get value
	res = fscanf(f, "%d", &val);

	if (res <= 0)
	{
		printf("...ERROR: read voltage input file.\n");
		fclose(f);

		return -1;
	}
	
	// Close file
	fclose(f);

	return val;
}

void a2d_task(void)
{
	int reading;
	long long prvs_cntr = 0;

	printf(" . . . Start a2d Task\n");

	while (progRun) 
	{
		reading = a2d_read_0();

		//Convert reading to array size
		if (reading == 0) 
		{
			pthread_mutex_lock(&arrayLen_mutex);
			gen_array_len = 1;
			pthread_mutex_unlock(&arrayLen_mutex);
		}
		if ((reading > 0) && (reading < 500))
		{
			pthread_mutex_lock(&arrayLen_mutex);
			gen_array_len = (reading * 19) / 500 + 1;
			pthread_mutex_unlock(&arrayLen_mutex);
		}
		else if ((reading >= 500) && (reading < 1000))
		{
			pthread_mutex_lock(&arrayLen_mutex);
			gen_array_len = (((reading - 500 ) * 40) / 500) + 20;
			pthread_mutex_unlock(&arrayLen_mutex);
		}
		else if ((reading >= 1000) && (reading < 1500))
		{
			pthread_mutex_lock(&arrayLen_mutex);
			gen_array_len = (((reading - 1000 ) * 60) / 500)+ 60;
			pthread_mutex_unlock(&arrayLen_mutex);
		}		
		else if ((reading >= 1500) && (reading < 2000))
		{
			pthread_mutex_lock(&arrayLen_mutex);
			gen_array_len = (((reading - 1500 ) * 130) / 500)+ 120;
			pthread_mutex_unlock(&arrayLen_mutex);
		}	
		else if ((reading >= 2000) && (reading < 2500))
		{
			pthread_mutex_lock(&arrayLen_mutex);
			gen_array_len = (((reading - 2000 ) * 50) / 500)+ 250;
			pthread_mutex_unlock(&arrayLen_mutex);
		}
		else if ((reading >= 2500) && (reading < 3000))
		{
			pthread_mutex_lock(&arrayLen_mutex);
			gen_array_len = (((reading - 2500 ) * 200) / 500)+ 300;
			pthread_mutex_unlock(&arrayLen_mutex);
		}
		else if ((reading >= 3000) && (reading < 3500))
		{
			pthread_mutex_lock(&arrayLen_mutex);
			gen_array_len = (((reading - 3000 ) * 300) / 500)+ 500;
			pthread_mutex_unlock(&arrayLen_mutex);
		}
		else if ((reading >= 3500) && (reading < 4000))
		{
			pthread_mutex_lock(&arrayLen_mutex);
			gen_array_len = (((reading - 3500 ) * 400) / 500)+ 800;
			pthread_mutex_unlock(&arrayLen_mutex);
		}
		else if ((reading >= 4000) && (reading <= 4100))
		{
			pthread_mutex_lock(&arrayLen_mutex);
			gen_array_len = (((reading - 4000 ) * 900) / 100)+ 1200;
			pthread_mutex_unlock(&arrayLen_mutex);
		}


		//Calculate the number of arrays sorted in last second
		cntr_num = sort_cntr - prvs_cntr;
		prvs_cntr = sort_cntr;

		//Sleep 1s, run this task once per second
    	nanosleep(&delay1s, (struct timespec *) NULL);
	}

	printf(" . . . Stop a2d Task\n");

	return;
}

/*
* Initialize and start a2d task
*/
void a2d_init()
{
	cntr_num = 0;
	gen_array_len = 100;

	a2d_set_bus();
	
	a2d_task();

    return;
}
