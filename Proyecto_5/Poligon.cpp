#include "Poligon.h"

void Scan_Line(void){
    int ymax = 0, ymin = 0;
    int intersec[128];
    int count;
    bool flag = false;

    // Loop para encontrar Ymax y Ymin
    for(int y = 0; y < YSIZE; y++){
        for(int x = 0; x < XSIZE; x++){
            if(matriz[y][x] == 1){
            }
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

    // Iteracion desde Ymax hasta Ymin para iniciar el 
    // rellenado del poligono
    for(int y = ymax; y <= ymin; y++){
        count = 0;  // Variable que almacena cuantos bordes se han encontrado
        // Bucle para determinar la cantidad de bordes y su posicion en x
        for(int x = 0; x < XSIZE-1; x++){
            if(matriz[y][x] && !matriz[y][x+1]){
                count += 1;
                intersec[count] = x;
            }
        }
        // Bucle para rellenar secciones entre bordes
        for(int i = 1; i <= count; i += 2){
            for(int x = intersec[i]; x < intersec[i+1]; x++){
                matriz[y][x] = 1;
            }
        }
    }
}

int InOut_Test(int x, int y){
    bool flag = false;
    int count = 0;
    int resultado = 0;
    // Revisa los pixeles en la fila y
    for(int i = 0; i <= x; i++){
        // Si encuentra un borde izquierdo, levanta el flag
        // Y aumenta el contador
        if(!matriz[y][i-1] && matriz[y][i]){
            flag = true;
            count += 1;
        }
        // Si encuentra un borde derecho y el flag esta levantado,
        // aumenta el contador y baja el flag
        else if(flag && matriz[y][i] && !matriz[y][i+1]){
            flag = false;
            count += 1;
        }
    }

    switch(count%2){
        case 0:     // Si el numero de intersecciones es par el punto esta fuera
            resultado = 0;
            break;
        case 1:     // Si el numero de intersecciones es impar el punto esta dentro
            resultado = 1;
            break;
        default:    // En caso de error retorna -1
            resultado = -1;
    }
    return resultado;
}