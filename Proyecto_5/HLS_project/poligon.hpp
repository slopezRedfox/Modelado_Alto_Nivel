#include <iostream>
#include <hls_stream.h>
#include <stdint.h>
#include <fstream>
#include <stdlib.h>

#define YSIZE 256
#define XSIZE 256

static int result = 0;
static int value = 0;
static int flag = 0;
static int in_matriz[256][256]  = {{0},{0}};
static int out_matriz[256][256]  = {{0},{0}};

int poligon_filling(int pixel_x, int pixel_y, int result, int in_x, int in_y, int in_value, int out_x, int out_y, int out_value);
int wrapper_poligon(int pixel_x, int pixel_y, int result, int in_x, int in_y, int in_value, int out_x, int out_y, int out_value);

template<typename T>
void input_matrix(T x, T y, T value){
	in_matriz[y][x] = value;
}
/*
template<typename T>
void print_matriz(){
	T test =0;
	for(int y = 0; y < 256; y ++){
			for(int x = 0; x < 256; x ++){
				std::cout<<out_matriz[y][x]<<" ";
			}
			std::cout<<std::endl;
		}
}
*/


template<typename T>
int output_matrix(T y, T x){
	value = out_matriz[y][x];
	return value;
	}

template<typename T>
void Scan_Line(){
		T ymax = 0, ymin = 0;
		T intersec[128];
		T count;
		bool flag = false;
		// Loop para encontrar Ymax y Ymin
			for(int y = 0; y < 256; y++){
				for(int x = 0; x < 256; x++){
					if(in_matriz[y][x] == 1){
					}
					// Cuando encuentra el primer pixel con valor 1
					// almacena el valor de y en ymax
					if(!flag&&in_matriz[y][x]){
						ymax = y;
						flag = true;
						break;
					}
					// Almacena los valores de y siguientes en ymin
					// por lo que al llegar al final el valor de y
					// sera ymin
					else if(in_matriz[y][x]){
						ymin = y;
						break;
					}
				}
			}

	   // Funcion para rellenar el poligono desde ymax hasta ymin
		for(int y = ymax; y <= ymin; y++){
				count = 0;
				for(int x = 0; x < 256-1; x++){
					if(in_matriz[y][x] && !in_matriz[y][x+1]){
						count += 1;
						intersec[count] = x;

					}
				}
				for(int i = 1; i <= count; i += 2){
					for(int x = intersec[i]; x < intersec[i+1]; x++){
						out_matriz[y][x] = 1;
					}
				}
		}
}

template<typename T>
int InOut_Test(T x, T y){
		bool flag = false;
		int count = 0;
		// Revisa los pixeles en la fila y
		for(int i = 0; i <= x; i++){
				if(!in_matriz[y][i-1] && in_matriz[y][i]){
					flag = true;
					count += 1;
				}
				else if(flag && in_matriz[y][i] && !in_matriz[y][i+1]){
					flag = false;
					count += 1;
				}
			}

		// Si el numero de intersecciones es par el punto esta fuera del poligono y
		// si es impar esta dentro
		switch(count%2){
			case 0:
				result = 0;
				break;
			case 1:
				result = 1;
				break;
			default:
				result = -1;
				//cout << "Error al realizar el test!" << endl;
		}
		return result;
};
