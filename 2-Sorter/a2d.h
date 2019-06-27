#ifndef _A2D_H_
#define _A2D_H_

extern int cntr_num; 
extern int gen_array_len;

//Write BB-ADC to slots file to load cape
int a2d_set_bus(void);

//Read value of potentiometer,
int a2d_read_0(void);

//The main task of a2d module. Read value from potentiometer
//and compute the number of arrays sorted every second
void a2d_task(void);

//Module initialization
void a2d_init(void);

#endif