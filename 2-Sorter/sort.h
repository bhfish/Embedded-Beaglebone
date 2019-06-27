#ifndef _SORT_H_
#define _SORT_H_

#include <stdbool.h>
#include <pthread.h>


extern int array_len;
extern long long sort_cntr;
extern int* array_sort;
extern bool array_ready;


//The main task of sorter module. It get an array length and dynamically 
//generate and fill array with random numbers, and sort. Then free the array.
void sorter_task(void);

//Free array being sorted
void sorterFreeArray(void);

//Function to sort array using bubble sort
void sorterSortArray(int length);

//Generate random number of given length and fill the array
void sorterGenArray(int length);

// Get the number of arrays which have finished being sorted.
long long Sorter_getNumberArraysSorted(void);

//Module initialization
void sorter_init();

#endif