#ifndef _POLIGON_H_
#define _POLIGON_H_

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
using namespace std;

#define YSIZE 256
#define XSIZE 256

int matriz[YSIZE][XSIZE];

void Scan_Line(void);

int InOut_Test(int, int);

#endif