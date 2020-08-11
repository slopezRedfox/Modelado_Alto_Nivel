#ifndef ADC_H
#define ADC_H

#include <systemc-ams>

#define Nbits 16
#define DATARATE 1

SCA_TDF_MODULE( ADC ) {
    //Entradas
    sca_tdf::sca_in<double> In;         //Entrada del ADC
    //sca_tdf::sca_in<double> Rf;         //Referencia se refiere al Voltaje maximo que permite el ADC
    sca_tdf::sca_de::sca_out<int> bits; //Salida del ADC en Hex
    sca_tdf::sca_de::sca_out<double> data; //Salida del ADC en Hex

    void set_attributes() {
        In.set_rate(DATARATE);
        //bits.set_timestep(1, sc_core::SC_NS);
    };

    void initialize() {};

    void processing() {
        bool   Out_Aux[Nbits];
        double Inp_Aux  = In.read();
        int    bits_Aux = 0;

        double Rf      = 40; //Arbitrariamente se coloca la referencia en 40V
        double Ref_Aux = Rf/pow(2,Nbits);
        
        //Ejemplo, Vmax a 40V con 4 bits
        /*
        37.5 - 40.0 = 1111
        35.0 - 37.5 = 1110
        32.5 - 35.0 = 1101
        30.0 - 32.5 = 1100
        27.5 - 30.0 = 1011
        25.0 - 27.5 = 1010
        22.5 - 25.0 = 1001
        20.0 - 22.5 = 1000
        17.5 - 20.0 = 0111
        15.0 - 17.5 = 0110
        12.5 - 15.0 = 0101
        10.0 - 12.5 = 0100
         7.5 - 10.0 = 0011
         5.0 -  7.5 = 0010
         2.5 -  5.0 = 0001
         0.0 -  2.5 = 0000

        */
        //std::cout << "Vref: " << Ref_Aux << std::endl;

        //--------------------------------------------------
        //Se hace un for desde N-1 bits desde el MSB al LSB 
        for (int i = Nbits-1; i >= 0; i--)
        {
            if(Inp_Aux >= (Ref_Aux*pow(2,i)))
            {
                Out_Aux[i] = 1;
            }
            else
            {
                Out_Aux[i] = 0;
            }
            Inp_Aux = Inp_Aux - (Ref_Aux*pow(2,i))*Out_Aux[i];
        }

        for (int j = 0; j < Nbits; j++)
        {
            bits_Aux = bits_Aux + Out_Aux[j]*pow(2,j);
        }

        bits.write(bits_Aux);
        data.write(bits_Aux*40/pow(2,Nbits));
    }

    void ac_processing() {};

    SCA_CTOR( ADC ) : In("In"){}
};

#endif