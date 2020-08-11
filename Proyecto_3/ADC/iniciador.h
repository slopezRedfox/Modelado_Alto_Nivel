#ifndef INIT_H
#define INIT_H

#include <systemc.h>   
using namespace sc_core;   
using namespace sc_dt;   
using namespace std;   
   
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

// User-defined extension class
struct ID_extension: tlm::tlm_extension<ID_extension> {
  ID_extension() : transaction_id(0) {}
  virtual tlm_extension_base* clone() const { // Must override pure virtual clone method
    ID_extension* t = new ID_extension;
    t->transaction_id = this->transaction_id;
    return t;
  }

  // Must override pure virtual copy_from method
  virtual void copy_from(tlm_extension_base const &ext) {
    transaction_id = static_cast<ID_extension const &>(ext).transaction_id;
  }
  unsigned int transaction_id;
};
   
struct Initiator: sc_module   
{   
  // TLM2 socket, defaults to 32-bits wide, generic payload, generic DMI mode
  tlm_utils::simple_initiator_socket<Initiator> socket; 

  SC_HAS_PROCESS(Initiator);
  Initiator(sc_module_name initiator)   // Construct and name socket   
  {   
    // Register callbacks for incoming interface method calls
    socket.register_nb_transport_bw(this, &Initiator::nb_transport_bw_adc);
    SC_THREAD(Muestreo_adc);
    SC_THREAD(thread_process_adc);   
  }   
   
  void thread_process_adc()   
  {
    tlm::tlm_generic_payload trans;
    sc_time delay = sc_time(10, SC_NS);   

    ID_extension* id_extension = new ID_extension;
    trans.set_extension( id_extension ); // Add the extension to the transaction
     
    while (true) 
    {  
      wait(do_t);
      
      tlm::tlm_phase phase = tlm::BEGIN_REQ;   
          
      tlm::tlm_command cmd = static_cast<tlm::tlm_command>(cmd_adc);   
      trans.set_command( cmd );
      trans.set_address( addrs_adc );   
      trans.set_data_ptr( reinterpret_cast<unsigned char*>(&data_adc) );   
      trans.set_data_length( 4 );   
  
      // Other fields default: byte enable = 0, streaming width = 0, DMI_hint = false, no extensions   
    
      //Delay for BEGIN_REQ
      wait(10, SC_NS);
      tlm::tlm_sync_enum status;   
    
      cout << endl << endl;
      cout << name() << " BEGIN_REQ SENT" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      status = socket->nb_transport_fw( trans, phase, delay );  // Non-blocking transport call   
  
      // Check value returned from nb_transport   
  
      switch (status)   
      {   
      case tlm::TLM_ACCEPTED:   
      
        wait(10, SC_NS);
        cout << name() << " END_REQ SENT" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

        phase = tlm::END_REQ; 
        status = socket->nb_transport_fw( trans, phase, delay );  // Non-blocking transport call
        break;   
  
      case tlm::TLM_UPDATED:   
      case tlm::TLM_COMPLETED:   
  
        // Initiator obliged to check response status   
        if (trans.is_response_error() )   
          SC_REPORT_ERROR("TLM2", "Response error from nb_transport_fw");   
  
        cout << "trans/fw = { " << (cmd ? 'W' : 'R') << ", " << hex << data_adc << " } , data = "   
              << hex << data_adc << " at time " << sc_time_stamp() << ", delay = " << delay << endl;   
        break;   
      }
      id_extension->transaction_id++; 
    }   
  }   
   
  // *********************************************   
  // TLM2 backward path non-blocking transport method   
  // *********************************************   
   
  virtual tlm::tlm_sync_enum nb_transport_bw_adc( tlm::tlm_generic_payload& trans,   
                                                  tlm::tlm_phase& phase, sc_time& delay )   
  {   
    tlm::tlm_command cmd = trans.get_command();   
    sc_dt::uint64    adr = trans.get_address();   
    
    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension ); 
    
    if (phase == tlm::END_RESP)
    {       
      wait(delay);      
      cout << name() << " END_RESP RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      return tlm::TLM_COMPLETED;
    }
    
    if (phase == tlm::BEGIN_RESP)
    {                          
      if (trans.is_response_error() )   
        SC_REPORT_ERROR("TLM2", "Response error from nb_transport");   
            
      cout << "trans/bw = { " << (cmd ? 'W' : 'R') << ", " << hex << adr   
           << " } , data = " << hex << data_adc << " at time " << sc_time_stamp()   
           << ", delay = " << delay << endl;
      
      wait(delay);
      done_t.notify();
      cout << name () << " BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      return tlm::TLM_ACCEPTED;   
    }

    else{
      return tlm::TLM_ACCEPTED;   
    }
  }   
  
  //====================================================================
  void Muestreo_adc()
  {
    while(true)
    {
      wait(50, SC_NS);
      if (flag_adc == 1)
      {
        addrs_adc = 0x0;
        data_adc  = 0x1;
        cmd_adc   = 1;
        do_t.notify();
        wait(done_t);
        flag_adc = 0;
      }
      
      else
      {
        addrs_adc = 0x1;
        data_adc  = 0x0;
        cmd_adc   = 0;
        do_t.notify();
        wait(done_t);
      }
    }
  }

  // Internal data buffer used by initiator with generic payload   
  sc_event do_t, done_t;
  
  bool cmd_adc;
  bool flag_adc = 1;
  int  data_adc;
  long int addrs_adc;
};

#endif