#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#include "joystick.h"
#include "beatbox.h"
#include "audioCtrl.h"
#include "audioMixer.h"
#include "accelerometer.h"
#include "udpServer.h"

bool stopping = false;
static pthread_t processThreadId;

void process(void)
{
	bool debounce;

	while(!stopping)
	{
		debounce = false;

		if(JOYST_UP == joystkDirection)
		{
			//printf("joystick pushed up.\n");
			audioCtrlVolIncr(5);
			debounce = true;
		}
		else if(JOYST_DOWN == joystkDirection)	
		{
			//printf("joystick pushed down.\n");
			audioCtrlVolIncr(-5);
			debounce = true;
		}
		else if(JOYST_PUSH == joystkDirection)
		{
			//printf("joystick pushed center.\n");
			audioCtrlNextBeat();
			debounce = true;
		}
		else if(JOYST_RIGHT == joystkDirection)
		{
			//printf("joystick pushed right.\n");
			audioCtrlTempoIncr(5);
			debounce = true;
		}
		else if(JOYST_LEFT == joystkDirection)
		{
			//printf("joystick pushed right.\n");
			audioCtrlTempoIncr(-5);
			debounce = true;
		}

		//Waiting for the debounce of joystick
		while((JOYST_NONE != joystkDirection) && (debounce == true))
		{
			nanosleep(&delay1ms, NULL);
		}		
	}
}

void processCleanup(void)
{
	pthread_join(processThreadId, NULL);
}

void processInit(void)
{
	int rt;

	rt = pthread_create(&processThreadId, NULL, (void *)&process, NULL);
	
	if(rt)
	{
		printf("...Error: Process thread creation failed: %d\n", rt);
	}
}


int main(int argc, char *argv[])
{
	joystkInit();
	AudioMixer_init();
	audioCtrlInit();
	acclrmInit();
	udpServerInit();
	processInit();

	processCleanup();
	acclrmCleanup();
	audioCtrlCleanup();
	AudioMixer_cleanup();
	joystCleanup();
	udpServerCleanup();

	return 0;
}