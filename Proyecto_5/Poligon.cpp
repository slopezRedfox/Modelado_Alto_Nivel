#include "Poligon.h"


int matriz[YSIZE][XSIZE];

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

   /*
   // Funcion para rellenar el poligono desde ymax hasta ymin
   for(int y = ymax; y <= ymin; y++){
       flag = false;
       for(int x = 0; x < XSIZE-1; x++){
           if(!flag && matriz[y][x]){
               flag = true;
           }
           else if(flag && matriz[y][x] && !matriz[y][x+1]){
               flag = false;
           }
           else if(flag){
               matriz[y][x] = 1;
           }
       }
   }
   */

    for(int y = ymax; y <= ymin; y++){
        count = 0;
        for(int x = 0; x < XSIZE-1; x++){
            if(matriz[y][x] && !matriz[y][x+1]){
                intersec[count] = x;
                count += 1;
            }
        }
        for(int i = 0; i <= count; i += 2){
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
        if(matriz[y][i] && matriz[y][i+1]){
            count += 1;
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

int main(){
    ifstream inFile;
    ofstream outFile;
    char data;
    int xcount = 0, ycount = 0;
    int retval1 = 0, retval2 = 0;
    int resultado;

    // Apertura del archivo con los datos originales
    inFile.open("image.txt");
    // Verficacion del archivo abierto
    if (!inFile) {
        cerr << "Unable to open file datafile.txt";
        exit(1);   // call system to stop
    }
    // Llenado de la matriz pixeles con los datos del
    // archivo abierto
    while(inFile.get(data)){
        if(data != 10 && data != 13 ){
            if(xcount < XSIZE){
                matriz[ycount][xcount] = (int) data - 48;
                xcount += 1;
            }
            else{
                xcount = 0;
                ycount += 1;
                matriz[ycount][xcount] = (int) data - 48;
                xcount += 1;
            }
        }
    }
    // Cerrado del archivo con los datos originales
    inFile.close();

    // Apertura del archivo para guardar los resultados
    // del Inside Outside Test Odd Even Rule
    outFile.open("InOut_Test.txt");
    outFile << "x";
    outFile << setw(20) <<"y";
    outFile << setw(20) << "Resultado";
    outFile << endl;

    resultado = InOut_Test(38, 20);
    outFile << 20;
    outFile << setw(20) << 38;
    outFile << setw(20) << resultado;
    outFile << endl;

    resultado = InOut_Test(65, 62);
    outFile << 62;
    outFile << setw(20) << 65;
    outFile << setw(20) << resultado;
    outFile << endl;

    resultado = InOut_Test(181, 232);
    outFile << 232;
    outFile << setw(20) << 181;
    outFile << setw(19) << resultado;
    outFile << endl;

    resultado = InOut_Test(108, 154);
    outFile << 154;
    outFile << setw(20) << 108;
    outFile << setw(19) << resultado;
    outFile << endl;

    outFile.close();


    // Apertura del archivo para guardar la imagen
    // rellenada con el algoritmo Scan Line
    outFile.open("Image_Filled.txt");

    // Llamado al algoritmo Scan Line
    Scan_Line();

    // Almacenado de la matriz generada por el 
    // algoritmo Poligon Filling
    for(int y = 0; y < YSIZE; y++){
        for(int x = 0; x < XSIZE; x++){
            outFile << matriz[y][x];
        }
        outFile << endl;
    }

    // Cerrado del archivo de salida
    outFile.close();

    /*
    // Compare the results file with the golden results
	retval1 = system("diff --brief -w image.txt image.golden.txt");
	if (retval1 != 0){
		printf("Test failed  !!!\n"); 
		retval1 = 1;
	}
    else{
		printf("Test passed !\n");
    }
    */

    /*
    // Compare the results file with the golden results
	retval1 = system("diff --brief -w test.txt test.golden.txt");
	if (retval1 != 0){
		printf("Test failed  !!!\n"); 
		retval1 = 1;
	}
    else{
		printf("Test passed !\n");
    }
    */

    return (retval1 + retval2);
}