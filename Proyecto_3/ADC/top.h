#ifndef ADC_TOP_H
#define ADC_TOP_H

#include "iniciador.h"
#include "adc_tlm.h"

SC_MODULE(TOP)
{   
  Initiator *initiator;   
  Adc_tlm   *adc_tlm;   
  sc_core::sc_in<int> IO;
  
  SC_HAS_PROCESS(TOP);
  TOP(sc_module_name top)   // Construct and name socket 
  
  {   
    // Instantiate components   
    initiator = new Initiator("initiator");
    adc_tlm   = new Adc_tlm  ("adc_tlm");   
    adc_tlm->IO(IO);
      
    // Bind initiator socket to target socket   
    initiator->socket.bind(adc_tlm->socket);   
  }
};

#endif