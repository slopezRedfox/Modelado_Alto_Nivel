#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <fstream>


using namespace std;

# define M_PI           3.14159265358979323846  /* pi */
float Lambda = 3.99;                     			//Short-Circuit current
float Psi = 5.1387085e-6;                			//Is current (saturation)
float alpha = 0.625;                 				//Thermal voltage relation
float V_oc = 1/alpha*(log(Lambda/Psi));     	//Open circuit voltage
float V_mpp = 17.4;                  				//Maximum power point voltage
float I_mpp = 3.75;                  				//Maximum power point current
float P_mpp = 65.25;                 				//Maximum power 
float y = log(Lambda);              			//Short-Circuit logarithm
float b = log(Psi);                 			//Is current logarithm
float V_cte = 16.69;

float InputVoltage(float t){
	return (V_cte + (0.3 * V_cte * sin(2 * M_PI * 1000 * t)));
}

float InputCurrent(float t){
	return (Lambda - exp( alpha * InputVoltage(t) + b));
}


