#ifndef ADC_TLM_H
#define ADC_TLM_H

#include <systemc.h>   
using namespace sc_core;   
using namespace sc_dt;   
using namespace std;   
   
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
   
struct Adc_tlm: sc_module   
{   
  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm_utils::simple_target_socket<Adc_tlm> socket;
  sc_core::sc_in<int> IO;

  enum { SIZE_ADC = 2 };   
  const sc_time LATENCY;   
   
  SC_CTOR(Adc_tlm)   
  : socket("socket"), LATENCY(10, SC_NS)   
  {   
    // Register callbacks for incoming interface method calls
    socket.register_nb_transport_fw(this, &Adc_tlm::nb_transport_fw);
   
    // Initialize regs with random data   
    for (int i = 0; i < SIZE_ADC; i++)   
      mem[i] = 0x0;

    SC_THREAD(Muestreo);
    SC_THREAD(thread_process);   
  }   
  
  virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_time& delay )
  {
    sc_dt::uint64    adr = trans.get_address();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension ); 
    
    if(phase == tlm::END_REQ)
    {  
      wait(delay);
      cout << name() << " END_REQ RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      return tlm::TLM_COMPLETED;
    }

    if(phase == tlm::BEGIN_REQ)
    {
      // Search for errors
      if (byt != 0) {
        trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
        return tlm::TLM_COMPLETED;
      }

      trans_pending=&trans;
      phase_pending=phase;
      delay_pending=delay;
      
      wait(delay);
      cout << name() << " BEGIN_REQ RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      e1.notify();

      return tlm::TLM_ACCEPTED;
    }
    
    else
    {
      return tlm::TLM_COMPLETED;
    }
  }
  
  // *********************************************   
  // Thread to call nb_transport on backward path   
  // ********************************************* 
   
  void thread_process()  
  {   
    while (true) {
    
      wait(e1); 
   
      tlm::tlm_phase phase;   
      ID_extension* id_extension = new ID_extension;
      trans_pending->get_extension( id_extension ); 
      
      tlm::tlm_command cmd = trans_pending->get_command();   
      sc_dt::uint64    adr = trans_pending->get_address();   
      unsigned char*   ptr = trans_pending->get_data_ptr();   
      unsigned int     len = trans_pending->get_data_length();   
      unsigned char*   byt = trans_pending->get_byte_enable_ptr();   
      unsigned int     wid = trans_pending->get_streaming_width();   

      if (adr >= sc_dt::uint64(SIZE_ADC) || byt != 0 || wid != 0 || len > 4)   
        SC_REPORT_ERROR("TLM2", "Target does not support given generic payload transaction");   
      
      // Obliged to implement read and write commands   
      if ( cmd == tlm::TLM_READ_COMMAND )   
        memcpy(ptr, &mem[adr], len);   
      else if ( cmd == tlm::TLM_WRITE_COMMAND )   
        memcpy(&mem[adr], ptr, len);   
             
      // Obliged to set response status to indicate successful completion   
      trans_pending->set_response_status( tlm::TLM_OK_RESPONSE );  
      
      wait(1, SC_NS);
      delay_pending= sc_time(1, SC_NS);
      
      cout << name() << " BEGIN_RESP SENT" << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
      
      tlm::tlm_sync_enum status;
      phase = tlm::BEGIN_RESP;   
      status = socket->nb_transport_bw( *trans_pending, phase, delay_pending );   

      switch (status)   
        
        case tlm::TLM_ACCEPTED:   
          
          wait(1, SC_NS);
          cout << name() << " END_RESP SENT" << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
  
          phase = tlm::END_RESP; 
          socket->nb_transport_bw( *trans_pending, phase, delay_pending );
    }   
  } 
  
  void Muestreo()
  {
    while(true){
      wait(1, SC_NS);
      if(mem[0] == 1)
      {
        mem[1] = IO.read();
        ///
        cout << endl;
        cout << "Escritura en memoria Data I/O   :" << hex << IO.read() << endl;
        cout << "Escritura en memoria Data MEM[0]:" << hex << mem[0] << endl;
        cout << "Escritura en memoria Data MEM[1]:" << hex << mem[1] << endl;
        cout << endl;//*/
      }
      else
      {
        ///
        cout << endl;
        cout << "Escritura en memoria Data I/O   :" << hex << IO.read() << endl;
        cout << "Escritura en memoria Data MEM[0]:" << hex << mem[0] << endl;
        cout << "Escritura en memoria Data MEM[1]:" << hex << mem[1] << endl;
        cout << endl;//*/
      }
    }
  }

  int mem[SIZE_ADC];   
  sc_event  e1;
  tlm::tlm_generic_payload* trans_pending;   
  tlm::tlm_phase phase_pending;   
  sc_time delay_pending;
};

#endif