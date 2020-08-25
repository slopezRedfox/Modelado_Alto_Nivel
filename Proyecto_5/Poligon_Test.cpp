#include "Poligon.h"

int main(){
    ifstream inFile;
    ofstream outFile;
    int retval = 0;
    int matriz2[YSIZE][XSIZE];
    int data, xcount = 0, ycount = 0;

    // Apertura del archivo con los datos originales
    inFile.open("image.txt");
    // Verficacion del archivo abierto
    if (!inFile) {
        cerr << "Unable to open file datafile.txt";
        exit(1);   // call system to stop
    }
    // Llenado de la matriz pixeles con los datos del
    // archivo abierto
    while(inFile >> data){
        if(xcount < XSIZE){
            matriz[ycount][xcount] = data;
            xcount += 1;
        }
        else{
            xcount = 0;
            ycount += 1;
            matriz[ycount][xcount] = data;
        }
    }
    // Cerrado del archivo con los datos originales
    inFile.close();

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



}