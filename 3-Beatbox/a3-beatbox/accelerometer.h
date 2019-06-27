#ifndef _ACCELEROMETER_H_
#define _ACCELEROMETER_H_

#include <stdint.h>

#define REG_ACCLRM_STATUS     0x00
#define REG_ACCLRM_OUT_X_MSB  0x01
#define REG_ACCLRM_OUT_X_LSB  0x02
#define REG_ACCLRM_OUT_Y_MSB  0x03
#define REG_ACCLRM_OUT_Y_LSB  0x04
#define REG_ACCLRM_OUT_Z_MSB  0x05
#define REG_ACCLRM_OUT_Z_LSB  0x06
#define REG_ACCLRM_CTRL_REG1  0x2A

typedef enum
{
	x_pos_movement = 0,
	x_neg_movement,
	y_pos_movement,
	y_neg_movement,
	z_pos_movement,
	z_neg_movement,
	none_movement	
} acclrmXyzDir_t;


typedef struct
{
	int16_t x_axis_val;
	int16_t y_axis_val;
	int16_t z_axis_val;	
} acclrmXyz_t;

typedef struct
{
	unsigned char regStatus;
	unsigned char regMsbX;
	unsigned char regLsbX;	
	unsigned char regMsbY;
	unsigned char regLsbY;
	unsigned char regMsbZ;
	unsigned char regLsbZ;		
} acclrmReg0_7_t;


void acclrmCleanup(void);
void acclrmInit(void);

#endif