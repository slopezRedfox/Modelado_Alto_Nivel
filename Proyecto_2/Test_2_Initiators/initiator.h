#ifndef INICIADOR_H
#define INICIADOR_H

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include <queue>

// Initiator module generating generic payload transactions
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

// Controler
struct Controler: sc_module {
  
  tlm_utils::simple_initiator_socket<Controler> socket_initiator;
  tlm_utils::simple_target_socket<Controler>    socket_target;

  // Constructor de Cotroler
  SC_CTOR(Controler) : 
    socket_initiator("socket_initiator"),
    socket_target("socket_to_target")
  {
    //Se tienen las funciones TLM2
    socket_initiator.register_nb_transport_bw(this, &Controler::nb_transport_bw);
    socket_target.register_nb_transport_fw(this, &Controler::nb_transport_fw);

    //Se tiene las funciones recurrente
    SC_THREAD(thread_process_to_fw);
    SC_THREAD(thread_process_to_bw);  

    
    SC_THREAD(TB);
  }

  //==============================================================================================
  //                                   FUNCIONES DE INICIADOR
  //==============================================================================================

  //----------------------------------------------------------------------------------------------
  // Thread Iniciador
  void thread_process_to_bw(){
    
    tlm::tlm_generic_payload trans;
    sc_time delay = sc_time(10, SC_NS);

    ID_extension* id_extension = new ID_extension;
    trans.set_extension( id_extension );

    while(true){

      wait(do_t.default_event());

      //PHASE == BEGIN_REQ
      tlm::tlm_phase phase = tlm::BEGIN_REQ;
      tlm::tlm_command cmd = static_cast<tlm::tlm_command>(comando);
      
      //Parametros de trans
      trans.set_command( cmd );   
      trans.set_address( addrs );   
      trans.set_data_ptr( reinterpret_cast<unsigned char*>(&data) );
      trans.set_data_length( 8 );   
      trans.set_byte_enable_ptr( 0 );

      tlm::tlm_sync_enum status;
      cout  << "0 - "<< name() << " BEGIN_REQ  SENT    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

      wait(delay);
      status = socket_initiator->nb_transport_fw(trans, phase, delay );
      wait(delay);

      switch (status)
      {
        case tlm::TLM_ACCEPTED:   
          
          cout  << "0 - "<< name() << " END_REQ    SENT    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

          phase = tlm::END_REQ;
          status = socket_initiator->nb_transport_fw( trans, phase, delay );
          break;
      
        case tlm::TLM_UPDATED:

          //None to do

        case tlm::TLM_COMPLETED:   
      
          if (trans.is_response_error() )   
            SC_REPORT_ERROR("TLM2", "Response error from nb_transport_fw");   

          cout << endl;
          cout  << "0 - " << " TRANS ID " << id_extension->transaction_id << "trans/fw = { " << (cmd ? 'W' : 'R') << ", " << hex << 0 << " } , data = "   
                << hex << data << " at time " << sc_time_stamp() << ", delay = " << delay << endl;
          cout << endl;
          break;   
      }

      done_t.notify();
      id_extension->transaction_id++;
    }
  }
  
  //----------------------------------------------------------------------------------------------
   
  virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase,
                                              sc_time& delay ){
    
    tlm::tlm_command cmd    =  trans.get_command();   
    sc_dt::uint64    adr    =  trans.get_address();   
    int              data_p = *trans.get_data_ptr();

    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension ); 
    
    if (phase == tlm::BEGIN_RESP) 
    {                          
      // Initiator obliged to check response status   
      if (trans.is_response_error() )   
        SC_REPORT_ERROR("TLM2", "Response error from nb_transport");   

      cout << endl;
      cout << " TRANS ID " << id_extension->transaction_id << " 0 - " 
            << "trans/bw = { " << (cmd ? 'W' : 'R') 
            << ", "            << hex << adr   
            << " } , data = "  << hex   << data_p 
            << " at time "     << sc_time_stamp()   
            << ", delay = "    << delay         
            << endl;
      cout << endl;

      cout  << "0 - "<< name () << " BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

      return tlm::TLM_ACCEPTED;
    } 

    else if (phase == tlm::END_RESP)
    {       
      cout  << "0 - "<< name() << " END_RESP   RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      
      cout << endl;
      cout << endl;
      cout << "------------------------------------------------------------------------" << endl;
      cout << endl;
      cout << endl;
      count = count +1;
      aux2.notify();

      return tlm::TLM_COMPLETED;
    }

    else{
      return tlm::TLM_COMPLETED;
    }
  }   
  

  //==============================================================================================
  //                                   FUNCIONES DE TARGET
  //==============================================================================================

  //----------------------------------------------------------------------------------------------
  // Thread Target
  void thread_process_to_fw(){
      
      while (true) {
          
        wait(target_t); 
    
        tlm::tlm_phase phase;
        ID_extension* id_extension = new ID_extension;
        trans_pending->get_extension( id_extension ); 
        
        //Se extraen los atributos de la transaccion
        tlm::tlm_command cmd = trans_pending->get_command();
        sc_dt::uint64    adr = trans_pending->get_address();
        unsigned char*   ptr = trans_pending->get_data_ptr();   
        unsigned int     len = trans_pending->get_data_length();   
        unsigned char*   byt = trans_pending->get_byte_enable_ptr();   
        unsigned int     wid = trans_pending->get_streaming_width();   

        //Al igual que en el punto anterior se revisa que todos los datos de la transaccione esten bien
        if (byt != 0 || wid != 0 || len > 8)   
          SC_REPORT_ERROR("TLM2", "Target does not support given generic payload transaction");   
        
        //
        //                            IMPORTANTE
        //
        // Se pasan los parametros a las variables globales, dependiendo si se esta
        // en escritura o lectura

        // Obliged to set response status to indicate successful completion   
        trans_pending->set_response_status( tlm::TLM_OK_RESPONSE );  
        delay_pending= sc_time(10, SC_NS);

        //********************************************************************************
        // SE MODIFICA SEGUN LO QUE SE NECESITE CUANDO SE VA A HACER LECTURAS/ESCRITURAS
        //********************************************************************************
        if ( cmd == tlm::TLM_READ_COMMAND ){
          cout << endl;
          cout << "HI: " << name() << endl;
          cout << endl;
        }

        else if ( cmd == tlm::TLM_WRITE_COMMAND ){
          cout << endl;
          cout << "HI: " << name() << endl;
          cout << endl;
        }

        //********************************************************************************
        //********************************************************************************
        
        tlm::tlm_sync_enum status;
        phase  = tlm::BEGIN_RESP; 
        
        cout  << "1 - "   << name() << "    BEGIN_RESP SENT    " << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;

        wait( sc_time(10, SC_NS) );
        status = socket_target->nb_transport_bw( *trans_pending, phase, delay_pending );   
        wait( sc_time(10, SC_NS) );          

        switch (status)     
          case tlm::TLM_ACCEPTED:   
          
          cout  << "1 - "   << name() << "    END_RESP   SENT    " << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
          phase = tlm::END_RESP;
          
          socket_target->nb_transport_bw( *trans_pending, phase, delay_pending );
      }   
    }
  
  //----------------------------------------------------------------------------------------------

  virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, 
                                              sc_time& delay){
      
      //Estos son los la lista de atributos usados en el non-bloking.
      sc_dt::uint64    adr = trans.get_address();
      unsigned int     len = trans.get_data_length();
      unsigned char*   byt = trans.get_byte_enable_ptr(); //Se busca el bit enable de datos
      unsigned int     wid = trans.get_streaming_width(); //PREGUNTA: ???? (no se usa)

      //Se le coloca un ID de la transaccion
      ID_extension* id_extension = new ID_extension;
      trans.get_extension( id_extension ); //Se extrae el ID de la transaccion

      //----------------------------------------------------------------------------------------------
      // PRIMERA FASE DE LA TRANSACCION        
      
      if(phase == tlm::BEGIN_REQ){
          
          //Se revisa que todos los datos de la transaccione esten bien
          if (byt != 0 || wid != 0 || len > 8){  
              SC_REPORT_ERROR("TLM2", "Target does not support given generic payload transaction");   
          }

          // Se pasa los datos a las variables globales de la estructura
          // Que estan declaradas al final de la misma, para poder ser
          // trabajados en la funcion thread
          trans_pending = &trans;
          phase_pending = phase;
          delay_pending = delay;

          cout  << "1 - "   << name() << "    BEGIN_REQ  RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;            

          target_t.notify();
          return tlm::TLM_ACCEPTED;
      }

      //----------------------------------------------------------------------------------------------
      // SEGUNDA FASE DE LA TRANSACCION
      
      else if(phase == tlm::END_REQ)
      {
          cout  << "1 - "   << name() << "    END_REQ    RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
          return tlm::TLM_COMPLETED;
      }

      else
      {
          return tlm::TLM_COMPLETED;
      }
  }


  //==============================================================================================
  //                                   FUNCIONES TB
  //==============================================================================================

  
  void TB(){
    cout << endl;
    cout << endl;

    while(count < 2){
      //-------------------------------0
      comando = 1;
      data    = 0x00000AAA;
                //                              offset
                //  |      tag        |  index    | |
      addrs  = 0b00000000000000000000000000000000;
      addrs  = addrs | 0xCA00000000;
      
      do_t.notify(0,SC_NS);
      wait(done_t);
      wait(80, SC_NS);

      //-------------------------------0
      comando = 1;
      data    = 0x00F00FFB;
                //                              offset
                //  |      tag        |  index    | |
      addrs  = 0b00000000000000000000000000000001;
      addrs  = addrs | 0xCA00000000;
      
      do_t.notify(0,SC_NS);
      wait(done_t);
      wait(70, SC_NS);

      cout << "hola" << endl;
    }
  }
  
  
  // Internal data buffer used by initiator with generic payload
  sc_event_queue do_t;
  sc_event done_t,aux2;

  int      data   ;  
  long int addrs  ;
  bool     comando;

  bool Exe = true;

  //Variables de puerto Target
  sc_event  target_t; 
  tlm::tlm_generic_payload* trans_pending;   
  tlm::tlm_phase phase_pending;   
  sc_time delay_pending;

  int count=0;
};


#endif