#ifndef INITIATOR_H
#define INITIATOR_H

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"


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

// Initiator module generating generic payload transactions   
struct Initiator: sc_module {
  
  // TLM2 socket, defaults to 32-bits wide, generic payload, generic DMI mode   
  tlm_utils::simple_initiator_socket<Initiator> socket;

  // Construct and name socket
  SC_CTOR(Initiator) : socket("socket"){
       
    // Se tiene una funcion de iniciador
    socket.register_nb_transport_bw(this, &Initiator::nb_transport_bw);
    
    //Se tiene una funcion recurrente
    SC_THREAD(thread_process);   
    SC_THREAD(TB);   
  }

  void thread_process(){
    
    tlm::tlm_generic_payload trans;       //Transaccion TLM2
    sc_time delay = sc_time(10, SC_NS);   //Delay standar 10ns

    ID_extension* id_extension = new ID_extension; //Se crea un ID con la clase anterior
    trans.set_extension( id_extension );

    while(Exe == true){
      //Espera a que la funcion TB indique el comando y el dato a transmitir o leer
      wait(do_t);

      //PHASE == BEGIN_REQ
      tlm::tlm_phase phase = tlm::BEGIN_REQ;
      tlm::tlm_command cmd = static_cast<tlm::tlm_command>(comando); //comando = (0 read, 1 write)
      
      //Parametros de trans
      trans.set_command( cmd );   
      trans.set_address( addrs );   
      trans.set_data_ptr( reinterpret_cast<unsigned char*>(&data) ); //Este es un puntero
      trans.set_data_length( 4 );   
      trans.set_byte_enable_ptr( 0 );

      //Imprime que se inicia la transaccion
      wait( sc_time(10, SC_NS) );
      cout  << "0 - "<< name() << " BEGIN_REQ  SENT    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      
      //Ejecuta la funsion nb_transport_fw, pero esta esta en target:
      // La funcion resibe:
      //  1. La transaccion trans
      //  2. La phase en la que se encuentra la comunicacion (BEGIN_REQ)
      //  3. El delay que se creo a 10ns
      
      //Crea la variable de status donde se almacena la respuesta de la funcion nb_transport_fw
      tlm::tlm_sync_enum status;

      //PREGUNTA: Aqui espera a que haya una respuesta???
      status = socket->nb_transport_fw(trans, phase, delay );  // Non-blocking transport call   
    
      // Checkea el status de la transaccion   
      switch (status)
      {
        case tlm::TLM_ACCEPTED:   
          
          //Delay for END_REQ
          wait( sc_time(10, SC_NS) );
          
          cout  << "0 - "<< name() << " END_REQ    SENT    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
          // Expect response on the backward path  
          phase = tlm::END_REQ; 
          status = socket->nb_transport_fw( trans, phase, delay );  // Non-blocking transport call
          break;   
      
        case tlm::TLM_UPDATED:

          //None to do

        case tlm::TLM_COMPLETED:   
      
          // Initiator obliged to check response status   
          if (trans.is_response_error() )   
            SC_REPORT_ERROR("TLM2", "Response error from nb_transport_fw");   

          cout << endl;
          cout  << "0 - "<< "trans/fw = { " << (cmd ? 'W' : 'R') << ", " << hex << 0 << " } , data = "   
                << hex << data << " at time " << sc_time_stamp() << ", delay = " << delay << endl;
          cout << endl;
          
          break;   
      }
      
      //Delay between RD/WR request
      wait(aux);
      wait(100, SC_NS);   
      
      id_extension->transaction_id++;
      done_t.notify();
    }
  }
   
  // *********************************************   
  // TLM2 backward path non-blocking transport method   
  // *********************************************   
   
  virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase,
                                              sc_time& delay ){
    
    tlm::tlm_command cmd = trans.get_command();   
    sc_dt::uint64    adr = trans.get_address();   
    
    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension ); 
    
    if (phase == tlm::BEGIN_RESP) {
                              
      // Initiator obliged to check response status   
      if (trans.is_response_error() )   
        SC_REPORT_ERROR("TLM2", "Response error from nb_transport");   

      cout << endl;
      cout  << "0 - " 
            << "trans/bw = { " << (cmd ? 'W' : 'R') 
            << ", "            << hex << adr   
            << " } , data = "  << hex   << data 
            << " at time "     << sc_time_stamp()   
            << ", delay = "    << delay         
            << endl;
      cout << endl;

      //Delay para BEGIN_RESP
      wait(delay);
      cout  << "0 - "<< name () << " BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      return tlm::TLM_ACCEPTED;
    } 

    if (phase == tlm::END_RESP) {  
           
      //Delay for END_RESP
      wait(delay);
      cout  << "0 - "<< name() << " END_RESP   RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      aux.notify();
      return tlm::TLM_COMPLETED;
    }
  }   
  
  void TB(){
    comando = 1;
    data    = 0x0000000A;
          //                              offset
          //  |      tag        |  index    | |
    addrs  =0b000000000000000001000000000000000;
    do_t.notify();
    wait(done_t);
    cout << endl;
    cout << endl;
  
    comando = 1;
    data    = 0x0000000B;
          //                              offset
          //  |      tag        |  index    | |
    addrs  =0b100000000000000011000000000000000;
    do_t.notify();
    wait(done_t);
    cout << endl;
    cout << endl;

    comando = 1;
    data    = 0x0000000B;
          //                              offset
          //  |      tag        |  index    | |
    addrs  =0b000000000000000011000000000000000;
    do_t.notify();
    wait(done_t);
    cout << endl;
    cout << endl;

    comando = 1;
    data    = 0x0000000A;
          //                              offset
          //  |      tag        |  index    | |
    addrs  =0b000000000000000111000000000000000;
    do_t.notify();
    wait(done_t);
    cout << endl;
    cout << endl;

    comando = 1;
    data    = 0x0000000C;
          //                              offset
          //  |      tag        |  index    | |
    addrs  =0b000000000000011111000000000000000;
    do_t.notify();
    wait(done_t);
    cout << endl;
    cout << endl;
  
    comando = 0;
    data    = 0x0000000C;
          //                              offset
          //  |      tag        |  index    | |
    addrs  =0b000000000000000011000000000000000;
    do_t.notify();
    wait(done_t);
    cout << endl;
    cout << endl;

/*
    comando = 1;
    data    = 0x0000000C;
          //                              offset
          //  |      tag        |  index    | |
    addrs  =0b100000000000000111000000000000000;
    do_t.notify();
    wait(done_t);
    cout << endl;
    cout << endl;

    comando = 1;
    data    = 0x0000000D;
          //                              offset
          //  |      tag        |  index    | |
    addrs  =0b100000000000001111000000000000000;
    do_t.notify();
    wait(done_t);
    cout << endl;
    cout << endl;

    comando = 0;
    data    = 0x0000000B;
          //                              offset
          //  |      tag        |  index    | |
    addrs  =0b100000000000000011000000000000000;
    do_t.notify();
    wait(done_t);
    cout << endl;
    cout << endl;

  //*/


    Exe = false;
  }
  
  
  // Internal data buffer used by initiator with generic payload
  sc_event  do_t, done_t, aux;
  int data;  
  long int addrs;
  bool comando;
  bool Exe = true;
};


#endif
