#ifndef ADC_H
#define ADC_H

#include <systemc-ams>


SCA_TDF_MODULE( ADC ) {
    //Entradas
    sca_tdf::sca_in<double> In; //Entrada del ADC
    sca_tdf::sca_in<double> Rf; //Referencia se refiere al Voltaje maximo que permite el ADC

    //Salidas, de momento solo 2 bits
    sca_tdf::sca_de::sca_out<bool> bit0;
    sca_tdf::sca_de::sca_out<bool> bit1;

    void set_attributes() {};

    void initialize() {};

    void processing() {
        double Inp_Aux = abs(In.read());
        double Ref_Aux = Rf.read()/pow(2,2);
        bool   Out_Aux[2];
        
        //Ejemplo, Vmax a 3.3 con 2 bits
        /*
        2.475 - 3.300 = 11
        1.650 - 2.475 = 10
        0.825 - 1.650 = 01
        0.000 - 0.825 = 00
        */
        //std::cout << "Vref: " << Ref_Aux << std::endl;

        //--------------------------------------------------
        //Se hace un for desde N-1 bits desde el MSB al LSB 
        for (int i = 1; i >= 0; i--)
        {
            if(Inp_Aux > (Ref_Aux*pow(2,i)))
            {
                Out_Aux[i] = 1;
            }
            else
            {
                Out_Aux[i] = 0;
            }
            Inp_Aux = Inp_Aux - (Ref_Aux*pow(2,i))*Out_Aux[i];
        }
        bit0.write(Out_Aux[0]);
        bit1.write(Out_Aux[1]);
    }

    void ac_processing() {};

    SCA_CTOR( ADC ) : In("In"), Rf("Rf"){}
};

#endif