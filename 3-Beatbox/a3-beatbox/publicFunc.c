#include "publicFunc.h"

const struct timespec delay1s   = {1, 0};
const struct timespec delay100ns  = {0, 100};
const struct timespec delay1ms  = {0, 1000000};
const struct timespec delay10ms = {0, 10000000};
const struct timespec delay50ms = {0, 50000000};
const struct timespec delay100ms= {0, 100000000};
const struct timespec delay200ms= {0, 200000000};

int fileWriteD(char* filePath, void* value)
{
	FILE *pfile = NULL;
	int res = 0;

	pfile = fopen(filePath, "w");

	if (pfile == NULL) 
	{
		printf("ERROR: Unable to open %s for write.\n", filePath);
		return -1;
	}

	if (fprintf(pfile, "%d", *(int*)value) <= 0) 
	{ 
		printf("ERROR: Writing data to %s.\n", filePath);
		res = -1;
	}

	fclose(pfile);
	return res;
}


int fileWriteS(char* filePath, void* value)
{
	FILE *pfile = NULL;
	int res = 0;

	pfile = fopen(filePath, "w");

	if (pfile == NULL) 
	{
		printf("ERROR: Unable to open %s for write.\n", filePath);
		return -1;
	}

	if (fprintf(pfile, "%s", (char*)value) <= 0) 
	{ 
		printf("ERROR: Writing data to %s.\n", filePath);
		res = -1;
	}

	fclose(pfile);
	return res;
}


int fileReadD(char* filePath)
{
	FILE *pfile = NULL;
	int value;

	pfile = fopen(filePath, "r");

	if (pfile == NULL) 
	{
		printf("ERROR: Unable to open %s for read.\n", filePath);
		return -1;
	}

	//Get value and conver char to int
	value = fgetc(pfile) - '0';

	fclose(pfile);
	return value;
}