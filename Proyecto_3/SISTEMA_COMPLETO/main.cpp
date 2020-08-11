#include <systemc-ams>

#include "adc.h"
#include "fuente_v.h"
#include "fuente_i.h"
#include "top.h"

int sc_main(int argc, char* argv[])
{
  //Top *top;
  //top = new Top("top");
  //sc_start(5000, sc_core::SC_NS);

  //Señales del ADC I (cables)
  sca_tdf::sca_signal<double> In_i;
  sc_core::sc_signal<int> bits_i;
  sc_core::sc_signal<double> data_i;

  //Señales del ADC V (cables)
  sca_tdf::sca_signal<double> In_v;
  sc_core::sc_signal<int> bits_v;
  sc_core::sc_signal<double> data_v;
  
  sc_core::sc_time time_step(10.0, sc_core::SC_NS);

  //=================================================
  //Inicializacion del ADC V
  ADC adcV("adcV");
  adcV.In(In_v);        //Entrada
  adcV.bits(bits_v);    //Salida bits (int)
  adcV.data(data_v);    //Salida data flotante (double)

  //Inicializacion de fuente
  v_sig v("v", time_step);
  v.out(In_v);

  //=================================================
  //Inicializacion del ADC I
  ADC adcI("adcI");
  adcI.In(In_i);        //Entrada
  adcI.bits(bits_i);    //Salida bits (int)
  adcI.data(data_i);    //Salida data flotante (double)

  //Inicializacion de fuente
  i_sig i("i", time_step);
  i.out(In_i);

  //=================================================
  //Inicializacion del Top (aqui esta el TLM)
  Top *top;
  top = new Top("top");
  //Se conecta la salida del ADC al IO del TLM
  top->IO_v(bits_v);
  top->IO_i(bits_i);

  //Salida a VCD
  sca_util::sca_trace_file *vcdfile= sca_util::sca_create_vcd_trace_file("output.vcd");
  sca_trace(vcdfile, bits_v, "bits_v");
  sca_trace(vcdfile, top->IO_v, "IO_v");
  sca_trace(vcdfile, data_v, "data_v");
  sca_trace(vcdfile, In_v, "In_v");

  sca_trace(vcdfile, bits_i, "bits_i");
  sca_trace(vcdfile, top->IO_i, "IO_i");
  sca_trace(vcdfile, data_i, "data_i");
  sca_trace(vcdfile, In_i, "In_i");

  sc_start(20, sc_core::SC_MS);

  sca_util::sca_close_vcd_trace_file(vcdfile);
  return 0;
}