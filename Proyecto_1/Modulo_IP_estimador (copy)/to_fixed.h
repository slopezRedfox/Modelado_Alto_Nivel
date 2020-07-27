#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define INT2U32(x) *(uint32_t*)&x

#define INT2U16(x) *(uint16_t*)&x

uint16_t to_fixed_16(float a){
	a=a*pow(2,16);
	int b = (int)a;
	return INT2U16(b);
}

uint32_t to_fixed_32(float a){
	a=a*pow(2,21);
	int b = (int)a;
	return INT2U32(b);
}

