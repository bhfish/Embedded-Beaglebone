#ifndef _PARENT_PUBLIC_FUNC_H_
#define _PARENT_PUBLIC_FUNC_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STR_CONVERT(str) #str

extern const struct timespec delay1s;
extern const struct timespec delay100ns;
extern const struct timespec delay1ms;
extern const struct timespec delay10ms;
extern const struct timespec delay50ms;
extern const struct timespec delay100ms;
extern const struct timespec delay200ms;

int fileWriteD(char* filePath, void* value);
int fileWriteS(char* filePath, void* value);
int fileReadD(char* filePath);


#endif