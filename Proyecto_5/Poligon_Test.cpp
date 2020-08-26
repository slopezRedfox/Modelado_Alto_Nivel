#include "Poligon.h"

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

    
    // Compare the results file with the golden results
	retval1 = system("diff --brief -w Image_Filled.txt Image_Filled.golden.txt");
	if (retval1 != 0){
		printf("Test Scan Line Poligon Filling failed  !!!\n"); 
		retval1 = 1;
	}
    else{
		printf("Test Scan Line Poligon Filling passed !\n");
    }
    

    
    // Compare the results file with the golden results
	retval2 = system("diff --brief -w InOut_Test.txt InOut_Test.golden.txt");
	if (retval2 != 0){
		printf("Inside Outside Test Even Odd Rule failed  !!!\n"); 
		retval2 = 1;
	}
    else{
		printf("Inside Outside Test Even Odd Rule passed !\n");
    }

    return (retval1 + retval2);
}