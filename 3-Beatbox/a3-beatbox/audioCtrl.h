#ifndef _AUDIO_CTRL_H_
#define _AUDIO_CTRL_H_

#include "audioMixer.h"

#define BEAT_MODE_ROCK     0
#define BEAT_MODE_CUSTOM   1
#define BEAT_MODE_NONE     2
#define BEAT_MODE_TOTAL    3

extern wavedata_t drumBeatHihat;
extern wavedata_t drumBeatSnare;
extern wavedata_t drumBeatBase;


void audioCtrlVolIncr(int incrVol);
void audioCtrlSetTemp(int tempo);
void audioCtrlSetVol(int vol);
void audioCtrlCleanup(void);
void audioCtrlInit(void);
void audioCtrlNextBeat(void);
void audioCtrlTempoIncr(int incr);
int audioCtrlGetTempo(void);
int audioCtrlGetVol(void);
void audioCtrlPlay(wavedata_t* sound);
int audioCtrlGetBeatMode(void);
void audioCtrlSetBeatMode(int mode);

#endif