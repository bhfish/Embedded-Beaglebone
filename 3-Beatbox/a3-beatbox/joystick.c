#include <stdio.h>
#include <stdlib.h>
#include "publicFunc.h"
#include "joystick.h"
#include "gpio.h"
#include <pthread.h>
#include "beatbox.h"

static pthread_t jsThreadId;

joystkDrctn_t joystkDirection;
/**
* Function to get joystick dirction
**/
static joystkDrctn_t getJsDrctn(void)
{

	if(0 == fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_JSUP)))
	{
		return JOYST_UP;
	}
	else if(0 == fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_JSDN)))
	{
		return JOYST_DOWN;
	}
	else if(0 == fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_JSRT)))
	{
		return JOYST_RIGHT;
	}
	else if(0 == fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_JSLFT)))
	{
		return JOYST_LEFT;
	}
	else if(0 == fileReadD(GPIO_VALUE_FILE_PATH(GPIO_PIN_JSPB)))
	{
		return JOYST_PUSH;
	}
	else
	{
		return JOYST_NONE;
	}	
}

/**
* The task to pull joystick dirction every 100ms
**/
static void joystkTask(void) 
{
	while(!stopping)
	{
		joystkDirection = getJsDrctn();
		nanosleep(&delay100ms, NULL);
	}
}

static int joystkGpioExport(void)
{
	int res = 0;
	int value;

	value = GPIO_PIN_JSUP;
	res = fileWriteD(GPIO_EXPORT, &value);

	value = GPIO_PIN_JSPB;
	if(res == 0)
	{
		res = fileWriteD(GPIO_EXPORT, &value);
	}

	value = GPIO_PIN_JSRT;
	if(res == 0)
	{
		res = fileWriteD(GPIO_EXPORT, &value);
	}

	value = GPIO_PIN_JSDN;
	if(res == 0)
	{
		res = fileWriteD(GPIO_EXPORT, &value);
	}

	value = GPIO_PIN_JSLFT;
	if(res == 0)
	{
		res = fileWriteD(GPIO_EXPORT, &value);
	}

	return res;
}

static int joystkGpioSetDrctnIn(void)
{
	int res = 0;
	char* value = "in";

	res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_JSUP), &value);

	if(res == 0)
	{
		res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_JSPB), &value);
	}

	if(res == 0)
	{
		res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_JSRT), &value);
	}

	if(res == 0)
	{
		res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_JSDN), &value);
	}

	if(res == 0)
	{
		res = fileWriteS(GPIO_DRCTN_FILE_PATH(GPIO_PIN_JSLFT), &value);
	}

	return res;
}


void joystCleanup(void)
{

	printf("...Stopping joystick thread.\n");
	pthread_join(jsThreadId, NULL);
}

/**
* Thread initialization
**/
void joystkInit(void)
{
	int res = 0;

	joystkDirection = JOYST_NONE;

	res = joystkGpioExport();

	if(res == 0)
	{
		res = joystkGpioSetDrctnIn();
	}

	if(res == 0)
	{
		printf("...Creating joystick thread.\n");
		res = pthread_create(&jsThreadId, NULL,  (void *)&joystkTask, NULL);
	}

    if( res )
	{
		printf("...Error: Thread creation failed: %d\n", res);	
	}
}