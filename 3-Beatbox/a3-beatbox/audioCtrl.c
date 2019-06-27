#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include "beatbox.h"
#include "audioCtrl.h"


#define DRUM_HIHAT "beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define DRUM_SNARE "beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"
#define DRUM_BASE  "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"


//BPM tempo defination
#define BPM_DEFAULT 120
#define BPM_MAX     300
#define BPM_MIN     40

//Volume defination
#define VOL_DEFAULT 80
#define VOL_MAX     100
#define VOL_MIN     0

//Beat mode defination
#define BEAT_PER_BAR       8
#define BEAT_ACTION_NUM    2 
#define BEAT_MODE_DEFAULT  (BEAT_MODE_ROCK)

#define BPM_DELAY_COEFF    (1000LL * 1000 * 1000 * 60 / 2)


wavedata_t drumBeatHihat;
wavedata_t drumBeatSnare;
wavedata_t drumBeatBase;


static int currentMode;
static int bpmTempo;
static int volLevel;
static pthread_t audioCtrlThreadId;

static pthread_mutex_t mutexSetVol;
static pthread_mutex_t mutexSetTempo;
static pthread_mutex_t mutexSetBeat;

wavedata_t* beatModeList[BEAT_MODE_TOTAL][BEAT_PER_BAR][BEAT_ACTION_NUM] = 
												{
												    {
												   	{&drumBeatHihat, &drumBeatSnare},
								  					{&drumBeatHihat, NULL},
								  					{&drumBeatHihat, &drumBeatBase},
					  				 				{&drumBeatHihat, NULL},
								  					{&drumBeatHihat, &drumBeatSnare},
								  					{&drumBeatHihat, NULL},
								  					{&drumBeatHihat, &drumBeatBase},
								  					{&drumBeatHihat, NULL}
								  				    },										    
												    {
												   	{&drumBeatHihat, &drumBeatBase},
								  					{&drumBeatHihat, &drumBeatBase},
								  				 	{&drumBeatHihat, NULL},
								  				 	{&drumBeatHihat, NULL},
								  				 	{&drumBeatHihat, &drumBeatSnare},
								  				 	{&drumBeatHihat, &drumBeatSnare},
								  				 	{&drumBeatHihat, NULL},
								  				 	{&drumBeatHihat, NULL}
								  				 	},
												    {
												   	{NULL, NULL},
								  					{NULL, NULL},
								  				 	{NULL, NULL},
								  				 	{NULL, NULL},
								  				 	{NULL, NULL},
								  				 	{NULL, NULL},
								  				 	{NULL, NULL},
								  				 	{NULL, NULL}
								  				 	},
												};

void audioCtrlCleanup(void)
{
	printf("...Stopping audio control thread.\n");
	pthread_join(audioCtrlThreadId, NULL);

	pthread_mutex_destroy(&mutexSetVol);
	pthread_mutex_destroy(&mutexSetTempo);
	pthread_mutex_destroy(&mutexSetBeat);
}


static void audioCtrlDelay(void)
{
	struct timespec delay;
	delay.tv_sec  = 0; 
	delay.tv_nsec = BPM_DELAY_COEFF / bpmTempo;
	nanosleep(&delay, (struct timespec *) NULL);
}


int audioCtrlGetTempo(void)
{
	return bpmTempo;
}

void audioCtrlSetTempo(int tempo)
{
	pthread_mutex_lock(&mutexSetTempo);

	if (tempo > BPM_MAX)
		bpmTempo = BPM_MAX;
	else if (tempo < BPM_MIN)
		bpmTempo = BPM_MIN;
	else
		bpmTempo = tempo;

	printf("...BPM: %d\n", bpmTempo);

	pthread_mutex_unlock(&mutexSetTempo);
}

void audioCtrlTempoIncr(int incr)
{
	audioCtrlSetTempo(audioCtrlGetTempo()+incr);
}

int audioCtrlGetVol(void)
{
	return volLevel;
}

void audioCtrlSetVol(int vol)
{
	pthread_mutex_lock(&mutexSetVol);

	if(vol > VOL_MAX)
		volLevel = VOL_MAX;
	else if(vol < VOL_MIN)
		volLevel = VOL_MIN;
	else
		volLevel = vol;

	printf("...Volume: %d\n", volLevel);
	AudioMixer_setVolume(volLevel);

	pthread_mutex_unlock(&mutexSetVol);
}

void audioCtrlVolIncr(int incrVol)
{
	audioCtrlSetVol(AudioMixer_getVolume() + incrVol);
}

void audioCtrlPlay(wavedata_t* sound)
{
	if(sound != NULL)
		AudioMixer_queueSound(sound);
}

void audioCtrlNextBeat(void)
{
	pthread_mutex_lock(&mutexSetBeat);

	if(currentMode == (BEAT_MODE_TOTAL - 1))
		currentMode = 0;
	else
		currentMode++;
	printf("...Play Beat: %d\n", currentMode);

	pthread_mutex_unlock(&mutexSetBeat);
}

int audioCtrlGetBeatMode(void)
{
	return currentMode;
}

void audioCtrlSetBeatMode(int mode)
{
	pthread_mutex_lock(&mutexSetBeat);

	if(mode < BEAT_MODE_TOTAL)
	{
		currentMode = mode;
	}
	else
	{
		printf("...Error: Setting mode %d.\n", mode);
	}

	pthread_mutex_unlock(&mutexSetBeat);
}


void audioCtrlTask(void)
{
	int beatSeq = 0;
	int mode = 0;

	while(!stopping)
	{
		//Load beat and play
		mode = currentMode;
		audioCtrlPlay(beatModeList[mode][beatSeq][0]);
		audioCtrlPlay(beatModeList[mode][beatSeq][1]);

		//Delay
		audioCtrlDelay();

		beatSeq++;
		if(beatSeq == BEAT_PER_BAR)
		{
			//Start from the first beat
			beatSeq = 0;
		}
	}

	//audioCtrlCleanup();
}

static void mutexInit(void)
{
    if (pthread_mutex_init(&mutexSetVol, NULL) != 0)
    {
        printf("...Error: Mutex mutexSetVol init failed\n");
    }

    if (pthread_mutex_init(&mutexSetTempo, NULL) != 0)
    {
        printf("...Error: Mutex mutexSetTempo init failed\n");
    }

    if (pthread_mutex_init(&mutexSetBeat, NULL) != 0)
    {
        printf("...Error: Mutex mutexSetBeat init failed\n");
    }        
}

void audioCtrlInit(void)
{
	int rt = 0;

	mutexInit();
	
	audioCtrlSetBeatMode(BEAT_MODE_DEFAULT);
	audioCtrlSetTempo(BPM_DEFAULT);
	audioCtrlSetVol(VOL_DEFAULT);

	AudioMixer_readWaveFileIntoMemory(DRUM_HIHAT, &drumBeatHihat);
	AudioMixer_readWaveFileIntoMemory(DRUM_SNARE, &drumBeatSnare);	
	AudioMixer_readWaveFileIntoMemory(DRUM_BASE, &drumBeatBase);

	printf("...Creating audio control thread.\n");
	rt = pthread_create(&audioCtrlThreadId, NULL,  (void *)&audioCtrlTask, NULL);
    if( rt )
	{
		printf("...Error: Audio control thread creation failed: %d\n", rt);
	}
}