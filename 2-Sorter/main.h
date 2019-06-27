#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdbool.h>
#include <pthread.h>

//The flag to continue/stop the program
extern bool progRun;

//The mutex
extern pthread_mutex_t arrayLen_mutex;
extern pthread_mutex_t arraySort_mutex;
extern pthread_mutex_t arrayRead_mutex;
extern pthread_mutex_t write_cape_slot_mutex;

//Stop the program
void programExit();

int main(int argc, char *argv[]);

#endif