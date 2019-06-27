/* 
* Modified based on the code from Dr. Brian Fraser.
*
* http://www.cs.sfu.ca/CourseCentral/433/bfraser/other/guide-code/segDisplay.c
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "i2c.h"
#include "publicFunc.h"
#include "beatbox.h"
#include "accelerometer.h"
#include "audioCtrl.h"

#define ACCLRM_Z_AXIS_G_OFFSET     1024
#define ACCLRM_XYZ_RAW_DATA_LEN       7
#define ACCLRM_RAW_DATA_THRESHOLD   330
#define ACCLRM_DIR_SLIDE_WINDIW      30
//#define RAW_REG_DATA_CONVERT(MSB, LSB)     ((((MSB) << 8) | (LSB)) / 16)


static int acclrmRegFd;
static pthread_t acclrmThreadId;
acclrmXyz_t axisData;

static void acclrmRawDataRead(acclrmXyz_t* xyz)
{
	acclrmReg0_7_t tempData;

	//Read first 7 accelerometer registers via I2C bus
	readI2cRegBurst(acclrmRegFd, REG_ACCLRM_STATUS, (char*)&tempData, sizeof(acclrmReg0_7_t));

	//Construct value of each axis
	xyz->x_axis_val = (int16_t)((tempData.regMsbX << 8) | (tempData.regLsbX)) / 16;
	xyz->y_axis_val = (int16_t)((tempData.regMsbY << 8) | (tempData.regLsbY)) / 16;
	xyz->z_axis_val = (int16_t)((tempData.regMsbZ << 8) | (tempData.regLsbZ)) / 16;
}

static acclrmXyzDir_t acclrmGetMoveDir(void)
{
	//acclrmXyz_t axisData;
	unsigned int absX;
	unsigned int absY;
	unsigned int absZ;

	//Read raw data from accelerometer's regs
	acclrmRawDataRead(&axisData);

	//Discount G on z-axis
	axisData.z_axis_val -= ACCLRM_Z_AXIS_G_OFFSET;

	//Convert to absolute value
	absX = abs(axisData.x_axis_val);
	absY = abs(axisData.y_axis_val);
	absZ = abs(axisData.z_axis_val);

	//Find the largest value among xyz directions
	if((absX > absY) && (absX > absZ) && (absX > ACCLRM_RAW_DATA_THRESHOLD))
	{
		if(axisData.x_axis_val>0)
			return x_pos_movement;
		else
			return x_neg_movement;
	}
	else if((absY > absX) && (absY > absZ) && (absY > ACCLRM_RAW_DATA_THRESHOLD))
	{
		if(axisData.y_axis_val>0)
			return y_pos_movement;
		else
			return y_neg_movement;
	}
	else if ((absZ > absX) && (absZ > absY) && (absZ > ACCLRM_RAW_DATA_THRESHOLD))
	{
		if(axisData.z_axis_val>0)
			return z_pos_movement;
		else
			return z_neg_movement;
	}
	else
	{
		return none_movement;
	}
}

static acclrmXyzDir_t acclrmGetDir(void)
{
	acclrmXyzDir_t axisMoveDir;

	//Get direction of movement
	axisMoveDir = acclrmGetMoveDir();

	//Detect debounce and movement time
	if(axisMoveDir != none_movement)
	{
		int count = 0;
		while(axisMoveDir == acclrmGetMoveDir())
		{
			count++;
			nanosleep(&delay1ms, NULL);
		}

		//Dismiss the movement if it's too short
		if(count < ACCLRM_DIR_SLIDE_WINDIW)
		{
			axisMoveDir = none_movement;
		}
		else
		{
			//we have detected movement and debounce here
			nanosleep(&delay100ms, NULL);
		}
	}

	return axisMoveDir;
}


static void acclrmTask(void)
{
	while(!stopping)
	{
		acclrmXyzDir_t movDir = acclrmGetDir();

		switch(movDir)
		{
			case x_pos_movement:
			case x_neg_movement:
				//printf("X-axis movement. x:%d y:%d z:%d\n", axisData.x_axis_val, axisData.y_axis_val, axisData.z_axis_val);
				audioCtrlPlay(&drumBeatHihat);
				nanosleep(&delay100ms, NULL);
				break;
			case y_pos_movement:
			case y_neg_movement:
				//printf("Y-axis movement. x:%d y:%d z:%d\n", axisData.x_axis_val, axisData.y_axis_val, axisData.z_axis_val);
				audioCtrlPlay(&drumBeatSnare);
				nanosleep(&delay100ms, NULL);
				break;
			case z_pos_movement:
			case z_neg_movement:
				//printf("Z-axis movement. x:%d y:%d z:%d\n", axisData.x_axis_val, axisData.y_axis_val, axisData.z_axis_val);
				audioCtrlPlay(&drumBeatBase);
				nanosleep(&delay100ms, NULL);
				break;
			case none_movement:
			default:
				break;							

		}
		nanosleep(&delay10ms, NULL);
	}
}

void acclrmInit(void)
{
	int res;

	//Configure I2C to enable accelerormeter
	acclrmRegFd = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS_ACCELEROMETER);
	
	writeI2cReg(acclrmRegFd, REG_ACCLRM_CTRL_REG1, 1);

	//Create thread detecting movement
	printf("...Creating accelerormeter thread.\n");
	res = pthread_create(&acclrmThreadId, NULL, (void *)&acclrmTask, NULL);

	if( res )
	{
		printf("...Error: Thread creation failed: %d\n", res);	
	}
}

void acclrmCleanup(void)
{
	printf("...Stoping accelerormeter thread.\n");
	pthread_join(acclrmThreadId, NULL);
	close(acclrmRegFd);
}