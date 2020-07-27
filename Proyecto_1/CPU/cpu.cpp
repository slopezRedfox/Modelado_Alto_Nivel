#include "systemc.h"

/*
Se plante un procesador en PV que realiza funciones y tareas necesarias para el funcionamiento 
del mismo.

Se plantean 2 salidas de direccion de 32 bits y read/write de 1 bit. 
Y una señal de 32 bits bidireccional que sea para los datos.

                        ----------------
                        |              |
                        |              |
                        |              |------> Rd_Wr
                        |              |------> Addr[31:0]
                        |     CPU      |
                        |              |<-----> Data[31:0]
                        |              |<------ Ready
                        |              |
                        |              |
                        ----------------


Este tiene como tarea inicial configurar el estimador, luego iniciar el mismo para que empiece a trabajar.
Y siempre se estra leyendo datos de memoria que corresponde al estimador para guardar los datos.


*/

//Direccion de registros de solo lectura del estimador
#define Param_approx_1_Addr 0x43c00010
#define Param_approx_2_Addr 0x43c00014
#define Voltage_out         0x43c00018
#define Current_out         0x43c0001c

//Direcciones de registro de solo escritura del estimador
#define I_scale_factor_Addr 0x43c00020
#define V_scale_factor_Addr 0x43c00024
#define Ig_value_Addr       0x43c00028
#define Gamma11_Addr        0x43c0002c
#define Gamma12_Addr        0x43c00030
#define Gamma21_Addr        0x43c00038
#define Gamma22_Addr        0x43c00040
#define Init_alpha_Addr     0x43c00048
#define Init_beta_Addr      0x43c00050
#define T_sampling_Addr     0x43c00058
#define Set_flag_Addr       0x43c00060

//Valores de los resgistro del estimador
#define I_scale_factor      1
#define V_scale_factor      2
#define Ig_value            3
#define Gamma11             4
#define Gamma12             5
#define Gamma21             6
#define Gamma22             7
#define Init_alpha          8
#define Init_beta           9
#define T_sampling          2
#define Set_flag            1

SC_MODULE (cpu) {

    int Data;

    SC_HAS_PROCESS(cpu);
        cpu (sc_module_name cpu):sc_module(cpu){
        }

    int write_data(int count){
        switch(count){

            case 0:
                return I_scale_factor;
                break;

            case 1:
                return V_scale_factor;
                break;

            case 2:
                return Ig_value;
                break;

            case 3:
                return Gamma11;
                break;

            case 4:
                return Gamma12;
                break;
            
            case 5:
                return Gamma21;
                break;

            case 6:
                return Gamma22;
                break;

            case 7:
                return Init_alpha;
                break;

            case 8:
                return Init_beta;
                break;

            case 9:
                return T_sampling;
                break;

            case 10:
                return Set_flag;
                break;
            
            default:
                return 0;
                break;
        }
    }

    int write_Addr(int count){
        switch(count){
            case 0:
                return I_scale_factor_Addr;
                break;

            case 1:
                return V_scale_factor_Addr;
                break;

            case 2:
                return Ig_value_Addr;
                break;

            case 3:
                return Gamma11_Addr;
                break;

            case 4:
                return Gamma12_Addr;
                break;
            
            case 5:
                return Gamma21_Addr;
                break;

            case 6:
                return Gamma22_Addr;
                break;

            case 7:
                return Init_alpha_Addr;
                break;

            case 8:
                return Init_beta_Addr;
                break;

            case 9:
                return T_sampling_Addr;
                break;

            case 10:
                return Set_flag_Addr;
                break;
            
            default:
                return 0;
                break;

        }
    }

    int read_addr(int count){
        switch (count)
        {
        case 0:
            return Param_approx_1_Addr;
            break;
        
        case 1:
            return Param_approx_2_Addr;
            break;

        case 2:
            return Voltage_out;
            break;

        case 3:
            return Current_out;
            break;

        default:
            return 0;
            break;
        }
    }


};