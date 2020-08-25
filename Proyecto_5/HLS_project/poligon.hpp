#include <iostream>
#include <hls_stream.h>
#include <stdint.h>
#include <fstream>
#include <stdlib.h>

#define YSIZE 256
#define XSIZE 256

void input_matrix(int x, int y, int value){
	int matrix[XSIZE][YSIZE] = {0};
	matrix[x][y] = value;
	
}

void Scan_Line(int matriz[YSIZE][XSIZE]){
    int ymax = 0, ymin = 0;
    int* p;
    int count;
    bool flag;

    // Loop para encontrar Ymax y Ymin
    for(int y = 0; y < YSIZE; y++){
        flag = false;
        for(int x = 0; x < XSIZE; x++){
            // Cuando encuentra el primer pixel con valor 1
            // almacena el valor de y en ymax
            if(!flag&&matriz[y][x]){
                ymax = y;
                flag = true;
                break;
            }
            // Almacena los valores de y siguientes en ymin
            // por lo que al llegar al final el valor de y
            // sera ymin
            else if(matriz[y][x]){
                ymin = y;
                break;
            }
        }
    }

    // Solo es necesario trabajar desde Ymax hasta Ymin
    for(int y = ymax; y <= ymin; y++){
        count = 0;
        // Aqui se asegura de almacenar las intersecciones 
        // en orden ascendente de x
        for(int x = 0; x < XSIZE; x++){
            if(matriz[y][x]){
                p[count] = x;
                count += 1;
            }
        }
        // Una vez se tienen almacenadas las intercecciones
        // se empieza a rellenar el poligono
        for(int i = 0; i <= count; i++){
            // Cuando count es impar significa que es un borde donde la izquierda
            // es interno y la derecha es externo
            if(!(count%2)){
                // Se rellena por lo tanto desde el borde anterior hasta el borde
                // actual, que corresponde el interior del poligono
                for(int x = p[count-1]; x <= p[count]; x++){
                    matriz[y][x] = 1;
                }
            }
        }
    }
}

void InOut_Test(int matriz[YSIZE][XSIZE], int x, int y){
    bool flag = false;
    int count = 0;
    // Revisa los pixeles en la fila y
    for(int i = 0; i <= x; i++){
        // Si el flag es false, significa que esta fuera del poligono al
        // encontrar un pixel de valor 1 aumenta la cuenta de intersecciones
        if(!flag&&matriz[y][i]){
            count += 1;
            flag = true;
        }
        // Si el pixel actual es 0 y el pixel anterior es 1, entonces el punto
        // anterior es una interseccion por lo que la cuenta aumenta en 1
        else if(!matriz[y][i]&&matriz[y][i-1]){
            count += 1;
            flag = false;
        }
    }

    // Si el numero de intersecciones es par el punto esta fuera del poligono y
    // si es impar esta dentro
    switch(count%2){
        case 0:
            //std::cout << "El punto esta afuera del poligono" << std::endl;
            break;
        case 1:
        	//std::cout << "El punto esta dentro del poligono" << std::endl;
            break;
        default:
        	//std::cout << "Error al realizar el test!" << std::endl;
    }
}