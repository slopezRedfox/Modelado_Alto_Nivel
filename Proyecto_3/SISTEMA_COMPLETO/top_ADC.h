#ifndef ADC_TOP_H
#define ADC_TOP_H

#include "iniciador.h"
#include "adc_tlm.h"

SC_MODULE(TOP)
{   
  Initiator *initiator_v;   
  Adc_tlm   *adc_v_tlm;   
  sc_core::sc_in<int> IO_v;

  Initiator *initiator_i;
  Adc_tlm   *adc_i_tlm;   
  sc_core::sc_in<int> IO_i;
  
  SC_HAS_PROCESS(TOP);
  TOP(sc_module_name top)   // Construct and name socket 
  
  {   
    // Coneccion de IP con ADC V 
    initiator_v = new Initiator("initiator_v"); //IP
    adc_v_tlm   = new Adc_tlm("adc_v_tlm");   
    adc_v_tlm->IO(IO_v);

    // Coneccion de IP con ADC I
    initiator_i = new Initiator("initiator_i"); //IP
    adc_i_tlm   = new Adc_tlm("adc_i_tlm");   
    adc_i_tlm->IO(IO_i);

    // Bind initiator_v socket to target socket   
    initiator_v->socket_adc.bind(adc_v_tlm->socket);   
    initiator_i->socket_adc.bind(adc_i_tlm->socket);
  }
};

#endif