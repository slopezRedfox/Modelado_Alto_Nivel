#include "Poligon.h"


void Scan_Line(void){
    int ymax = 0, ymin = 0;
    //int* p;
    //int count;
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

    /*
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
                    //put(matriz[y][x]);
                }
            }
        }
    }
    */

   // Funcion para rellenar el poligono desde ymax hasta ymin
   for(int y = ymax; y <= ymin; y++){
       flag = false;
       for(int x = 0; x < XSIZE-1; x++){
           if(!flag && matriz[y][x]){
               flag = true;
           }
           else if(flag&&matriz[y][x]&&!matriz[y][x+1]){
               flag = false;
           }
           if(flag){
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
            resultado = 0;
            break;
        case 1:
            resultado = 1;
            break;
        default:
            cout << "Error al realizar el test!" << endl;
    }
    return resultado;
}