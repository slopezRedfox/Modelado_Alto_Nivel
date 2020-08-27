#include <iostream>
#include <fstream>
#include <hls_stream.h>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include "poligon.hpp"



int matriz[256][256] = {{0},{0}};

int main(){
    std::ifstream inFile;
    std::ofstream outFile;
    char data;
    int xcount = 0, ycount = 0;
    int retval1 = 0, retval2 = 0;
    int resultado;

    int pixel_x = 0;
	int pixel_y = 0;
	int result = 0;
	int in_x = 0;
	int in_y = 0;
	int in_value = 0;
	int out_x = 0;
	int out_y  = 0;
	int out_value = 0;

    // Apertura del archivo con los datos originales
    //inFile.open("/home/local/ESTUDIANTES/rzarate/vivadoprjs/Modelado_Alto_Nivel/Proyecto_5/image.txt");
	inFile.open("/home/local/ESTUDIANTES/rzarate/vivadoprjs/Modelado_Alto_Nivel/Proyecto_5/image2.txt");
    // Verficacion del archivo abierto
    if (!inFile) {
    	std::cerr << "Unable to open file datafile.txt";
        exit(1);   // call system to stop
    }
    // Llenado de la matriz pixeles con los datos del
    // archivo abierto
    while(inFile.get(data)){
        if(data != 10 && data != 13 ){
            if(xcount < XSIZE){
                matriz[ycount][xcount] = (int) data - 48;
                //std::cout<<matriz[ycount][xcount]<< " ";
                xcount += 1;
            }
            else{

                xcount = 0;
                ycount += 1;
                matriz[ycount][xcount] = (int) data - 48;
                //std::cout<< std::endl<< matriz[ycount][xcount]<<" ";

                xcount += 1;
            }
        }
    }
    // Cerrado del archivo con los datos originales
    inFile.close();


    for(int y = 0; y < 256; y++){
            for(int x = 0; x < 256; x++){
            	//input_matrix<int>(x, y, matriz[y][x]);
            	wrapper_poligon(pixel_x, pixel_y, result, x, y, matriz[y][x], out_x, out_y, out_value);
            }
        }



    // Apertura del archivo para guardar los resultados
    // del Inside Outside Test Odd Even Rule
    outFile.open("/home/local/ESTUDIANTES/rzarate/vivadoprjs/Modelado_Alto_Nivel/Proyecto_5/InOut_Test.txt");
    outFile << "x";
    outFile << std::setw(20) <<"y";
    outFile << std::setw(20) << "Resultado";
    outFile << std::endl;

    resultado = InOut_Test<int>(38, 20);
    outFile << 20;
    outFile << std::setw(20) << 38;
    outFile << std::setw(20) << resultado;
    std::cout<<"Ressultado "<<resultado<<std::endl;
    outFile << std::endl;

    resultado = InOut_Test<int>(65, 62);
    outFile << 62;
    outFile << std::setw(20) << 65;
    outFile << std::setw(20) << resultado;
    std::cout<<"Ressultado "<<resultado<<std::endl;
    outFile << std::endl;

    resultado = InOut_Test<int>(181, 232);
    outFile << 232;
    outFile << std::setw(20) << 181;
    outFile << std::setw(19) << resultado;
    std::cout<<"Ressultado "<<resultado<<std::endl;
    outFile << std::endl;

    resultado = InOut_Test<int>(108, 154);
    outFile << 154;
    outFile << std::setw(20) << 108;
    outFile << std::setw(19) << resultado;
    std::cout<<"Ressultado "<<resultado<<std::endl;
    outFile << std::endl;

    outFile.close();


    // Apertura del archivo para guardar la imagen
    // rellenada con el algoritmo Scan Line
    outFile.open("/home/local/ESTUDIANTES/rzarate/vivadoprjs/Modelado_Alto_Nivel/Proyecto_5/Image_Filled.txt");

    // Llamado al algoritmo Scan Line
    Scan_Line<int>();


    // Almacenado de la matriz generada por el
    // algoritmo Poligon Filling
    int value;
    for(int y = 0; y < 256; y++){
        for(int x = 0; x < 256; x++){
        	value = output_matrix<int>(y,x);
        	//std::cout<<value<<std::endl;
            matriz[y][x] = value;
        }
    }

    //print_matriz<int>();
    for(int y = 0; y < 256; y++){
            for(int x = 0; x < 256; x++){
            	//std::cout<<matriz[y][x]<<" ";
                outFile << matriz[y][x];
            }
            outFile << std::endl;
            //std::cout<<std::endl;
        }

    // Cerrado del archivo de salida
    outFile.close();


    // Compare the results file with the golden results
	retval2 = system("diff --brief -w /home/local/ESTUDIANTES/rzarate/vivadoprjs/Modelado_Alto_Nivel/Proyecto_5/InOut_Test.txt /home/local/ESTUDIANTES/rzarate/vivadoprjs/Modelado_Alto_Nivel/Proyecto_5/InOut_Test2.golden.txt");
	if (retval2 != 0){
		printf("Inside Outside Test Even Odd Rule failed  !!!\n");
		retval2 = 1;
	}
    else{
		printf("Inside Outside Test Even Odd Rule passed !\n");
    }


    return 0;
}
