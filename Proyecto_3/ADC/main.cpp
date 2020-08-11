#include <systemc-ams>

#include "adc.h"
#include "fuente_v.h"
#include "top.h"

int sc_main (int argc, char* argv[])
{   
    //Señales del ADC (cables)
    sca_tdf::sca_signal<double> In;
    sc_core::sc_signal<int> bits;
    
    sc_core::sc_time time_step(1.0, sc_core::SC_NS);

    //Inicializacion del ADC
    ADC adcV("adcV");
    adcV.In(In);        //Entrada
    adcV.bits(bits);    //Salida bit 3

    //Inicializacion de fuente
    v_sig v("v", time_step);
    v.out(In);

    //Inicializacion del Top (aqui esta el TLM)
    TOP *top;
    top = new TOP("top");
    //Se conecta la salida del ADC al IO del TLM
    top->IO(bits);

    //Salida a VCD
    sca_util::sca_trace_file *vcdfile= sca_util::sca_create_vcd_trace_file("output.vcd");
    sca_trace(vcdfile, In, "In");
    sca_trace(vcdfile, bits, "bits");
    sca_trace(vcdfile, top->IO, "IO_request");

    sc_start(2, sc_core::SC_MS);

    sca_util::sca_close_vcd_trace_file(vcdfile);

	return 0;
}
