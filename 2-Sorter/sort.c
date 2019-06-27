#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort.h"
#include "main.h"
#include "a2d.h"

int array_len = 100;
long long sort_cntr = 0;
int* array_sort = NULL;
bool array_ready = false;

long long Sorter_getNumberArraysSorted(void)
{
	return sort_cntr;
}

int Sorter_getArrayLength(void)
{
	array_len = gen_array_len;
	return array_len;
}

void sorterGenArray(int length)
{

	int i,temp,index;

	array_sort =  (int*)malloc(length * sizeof(int));

	//random generate number 1 through size
	for(i = 0;i < length; i++){
		array_sort[i] = i + 1; 
	}
	//printlist(array_sort, length);
	//random swap 
	for(i = 0;i < length; i++){
		 index = rand() % length;
		 temp = array_sort[i];
		 array_sort[i] = array_sort[index];
		 array_sort[index] = temp;
		// printf("show here");
		 //printlist(array_sort, length);

	}

	return;
}

void sorterSortArray(int length)
{
	int i,j,temp;

	for(i = 0; i < length; i++){
		for(j = length-1; j > i; j--){
			if(array_sort[j]< array_sort[j-1]){

				pthread_mutex_lock(&arraySort_mutex);

				temp = array_sort[j];
				array_sort[j] = array_sort[j-1];
				array_sort[j-1] = temp;

				pthread_mutex_unlock(&arraySort_mutex);
			}

		}
	}

	sort_cntr++;
	return;
}

void sorterFreeArray(void)
{
	free(array_sort);
	array_sort = NULL;
	return;
}

void sorter_task(void)
{	
	printf(" . . . Start Sorter Task\n");

	//Lock 'arrayRead_mutex' to make sure the sorter array 
	//won't be read before it's ready
	pthread_mutex_lock(&arrayRead_mutex);
	while(progRun)
	{
		//Get array length
		Sorter_getArrayLength();

		//Generate random number and fill the array
		sorterGenArray(array_len);
		
		//Unlock mutex 'arrayRead_mutex' to indicate array is ready to be read
		pthread_mutex_unlock(&arrayRead_mutex);

		//Sort array
		sorterSortArray(array_len);
		
		//Lock 'arrayRead_mutex' to make sure array won't be read when we free
		//the array to avoid segmentation fault or other bad values
		pthread_mutex_lock(&arrayRead_mutex);

		//Free array;
		sorterFreeArray();
	}

	printf(" . . . Stop Sorter Task\n");	
}

/*
* Initialize and start sorter task
*/
void sorter_init()
{
	array_len = 100;
	sort_cntr = 0;
	array_sort = NULL;

	sorter_task();

	return;
}