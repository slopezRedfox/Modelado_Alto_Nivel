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
#define Start_Addr          0x43c00060

//Constans from memory

#define I_scale_factor    5
#define V_scale_factor    22
#define Ig                3.99
#define GAMMA11           0.1
#define GAMMA12           0
#define GAMMA21           0
#define GAMMA22           100
#define INIT_ALPHA        0.55
#define INIT_BETA         -13.0
#define T_SAMPLING        1e-6
#define Start             1

#define INT2U32(x) *(uint32_t*)&x

#define DELAY_CPU 5

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
    sc_time delay = sc_time(DELAY_CPU, SC_NS);

    ID_extension* id_extension = new ID_extension; //Se crea un ID con la clase anterior
    trans.set_extension( id_extension );

    while(true)
    {
      wait(do_t.default_event());

      tlm::tlm_phase phase = tlm::BEGIN_REQ;
      tlm::tlm_command cmd = static_cast<tlm::tlm_command>(comando); //comando = (0 read, 1 write)
      
      //Parametros de trans
      trans.set_command( cmd );   
      trans.set_address( addrs );   
      trans.set_data_ptr( reinterpret_cast<unsigned char*>(&data) ); //Este es un puntero
      trans.set_data_length( 4 );   
      trans.set_byte_enable_ptr( 0 );

      tlm::tlm_sync_enum status;

      wait(delay);
      cout  << "0 - "<< name() << " BEGIN_REQ  SENT    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

      status = socket_initiator->nb_transport_fw(trans, phase, delay );
      wait(delay);

      switch (status)
      {
        case tlm::TLM_ACCEPTED:   
          
          cout  << "0 - "<< name() << " END_REQ    SENT    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
          phase = tlm::END_REQ;

          status = socket_initiator->nb_transport_fw( trans, phase, delay );  // Non-blocking transport call
          break;   
      
        case tlm::TLM_UPDATED:

          //None to do

        case tlm::TLM_COMPLETED:   
      
          if (trans.is_response_error() )   
            SC_REPORT_ERROR("TLM2", "Response error from nb_transport_fw");   

          cout << endl;
          cout  << "0 - "<< "trans/fw = { " << (cmd ? 'W' : 'R') << ", " << hex << 0 << " } , data = "   
                << hex << data << " at time " << sc_time_stamp() << ", delay = " << delay << endl;
          cout << endl;
          
          break;   
      }

      //Delay between RD/WR request
      
      id_extension->transaction_id++;
      done_t_CPU.notify();
    }
  }
   
  //----------------------------------------------------------------------------------------------
   
  virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase,
                                              sc_time& delay ){
    
    tlm::tlm_command cmd    =  trans.get_command();   
    sc_dt::uint64    adr    =  trans.get_address();   
    
    memcpy(&data_read, trans.get_data_ptr(), 4);

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
            << " } , data = "  << hex   << data_read
            << " at time "     << sc_time_stamp()   
            << ", delay = "    << delay         
            << " TRANS ID " << id_extension->transaction_id << endl;
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
      End_Resp.notify();
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
        if (byt != 0 || wid != 0 || len > 4)   
          SC_REPORT_ERROR("TLM2", "Target does not support given generic payload transaction");   
        
        //
        //                            IMPORTANTE
        //
        // Se pasan los parametros a las variables globales, dependiendo si se esta
        // en escritura o lectura

        // Obliged to set response status to indicate successful completion   
        trans_pending->set_response_status( tlm::TLM_OK_RESPONSE );  
        delay_pending= sc_time(DELAY_CPU, SC_NS);

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
        
        wait( delay_pending );
        cout  << "1 - "   << name() << "    BEGIN_RESP SENT    " << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
        
        status = socket_target->nb_transport_bw( *trans_pending, phase, delay_pending );   

        wait( delay_pending );          

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
          if (byt != 0 || wid != 0 || len > 4){  
              SC_REPORT_ERROR("TLM2", "Target does not support given generic payload transaction");   
          }

          // Se pasa los datos a las variables globales de la estructura
          // Que estan declaradas al final de la misma, para poder ser
          // trabajados en la funcion thread
          trans_pending = &trans;
          phase_pending = phase;
          delay_pending = delay;

          cout  << "1 - "   << name() << "    BEGIN_REQ  RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;            

          // Se hace un notify de e1 para permitir a la funcion thread ejecutar
          target_t.notify();

          //Se envia que la transaccion ha sido aceptada
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


  uint32_t to_fixed_32(float a){
    a=a*pow(2,21);
    int b = (int)a;
    return INT2U32(b);
  }

  float write_data(int count){
  switch(count){
    case 0:
      return I_scale_factor;
      break;

    case 1:
      return V_scale_factor;
      break;

    case 2:
      return Ig;
      break;

    case 3:
      return GAMMA11;
      break;

    case 4:
      return GAMMA12;
      break;
    
    case 5:
      return GAMMA21;
      break;

    case 6:
      return GAMMA22;
      break;

    case 7:
      return INIT_ALPHA;
      break;

    case 8:
      return INIT_BETA;
      break;

    case 9:
      return T_SAMPLING;
      break;

    case 10:
      return Start;
      break;

    default:
      return 0;
      break;
  }
}

  int write_addr(int count){
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
      return Start_Addr;
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
  
  void TB(){
    cout << endl;
    cout << endl;

    sc_trace_file *wf = sc_create_vcd_trace_file("cpu");
    wf->set_time_unit(1, SC_NS);
    
    // Dump the desired signals
    sc_trace(wf, out[0], "param_1");
    sc_trace(wf, out[1], "param_2");
    sc_trace(wf, out[2], "volt");
    sc_trace(wf, out[3], "current");

    cout << "@" << sc_time_stamp() << endl;
    cout << "\nInicializador del estimador. \n";

    //*****
    for (int i = 0; i < 11; i++)
    {      
      //Comunicacion
      comando = 1;
      float data_float   = write_data(i);

      data   = to_fixed_32(data_float);
      addrs  = write_addr(i);
      addrs  = addrs | 0xCE00000000;
      cout << "Estimador ------------------------------"  << endl;
      cout << "Estimador Escritura de datos en Estimador" << endl;
      cout << "Estimador ********CPU******* addrs:    " << hex << addrs  << endl;
      cout << "Estimador ********CPU******* data    : " << data_float << endl;  
      cout << "Estimador ********CPU******* data hex: " << hex << data << endl;
      do_t.notify(0,SC_NS);
      wait(done_t_CPU);
      wait(80,SC_NS);
    }

    wait(360, SC_NS);
    cout << "Estimador "  << endl;
    cout << "Estimador "  << endl;
    cout << "Estimador " << endl;
    cout << "Estimador Lectura de registro del estimador\n";
    while(true)
    {
      for (int i = 0; i < 4; i++)
      {
        //Lectura
        comando = 0;
        data    = 0;
        addrs  = read_addr(i);
        addrs  = addrs | 0xCA00000000;

        cout << "Estimador CPU " << endl;
        cout << "Estimador CPU p" << i+1 << " read  addrs   : " << hex  << addrs  << endl;

        do_t.notify(0,SC_NS);
        wait(End_Resp);

        cout << "Estimador " << endl;
        cout << "Estimador CPU p" << i+1 << " read  data    : " << (float)data/pow(2,21) << endl;
        cout << "Estimador CPU p" << i+1 << " read  data hex: " << data << endl;

        //Escritura en VCD
        out[i] = (float)data/pow(2,21);

        //Escritura
        comando = 1;
        data    = data;
        addrs   = random_addrs;
        addrs   = addrs | 0xCA00000000;

        cout << "Estimador CPU " << endl;
        cout << "Estimador CPU p" << i+1 << " write addrs   : " << hex << addrs  << endl;
        cout << "Estimador CPU p" << i+1 << " write data    : " << out[i] << endl;
        cout << "Estimador CPU p" << i+1 << " write data hex: " << data << endl;

        do_t.notify(0,SC_NS);
        wait(End_Resp);
        
        if (random_addrs == 0x40000){
          random_addrs = 0;
        }
        else{
          random_addrs++;
        }
      }
      wait(50, SC_NS); //delay entre cada lectura de memoria
    }
  }
  
  
  // Internal data buffer used by initiator with generic payload
  sc_event_queue do_t;
  sc_event  done_t_CPU, aux, End_Resp;
  int data, data_read;
  long int addrs;
  bool comando;
  
  bool Exe = true;

  //Variables de puerto Target
  sc_event  target_t; 
  tlm::tlm_generic_payload* trans_pending;   
  tlm::tlm_phase phase_pending;   
  sc_time delay_pending;

  //CPU
  float out[4];
  int random_addrs = 0;
};


#endif