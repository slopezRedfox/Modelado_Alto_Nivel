#ifndef ESTIMADOR_H
#define ESTIMADOR_H

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <iostream>
#include <fstream>

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#define calc_delay 0
//Constans from memory

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

/*
#define I_scale_factor_e    5
#define V_scale_factor_e    22
#define Ig_e                3.99
#define GAMMA11_e           0.1
#define GAMMA12_e           0
#define GAMMA21_e           0
#define GAMMA22_e           100
#define INIT_ALPHA_e        0.55
#define INIT_BETA_e         -13.0
#define T_SAMPLING_e        1e-6
*/
#define INT2U32(x) *(uint32_t*)&x
#define INT2U16(x) *(uint16_t*)&x

# define M_PI           3.14159265358979323846  /* pi */

#define DELAY_IP 5

// Estimador
struct Estimador: sc_module {
  
  tlm_utils::simple_initiator_socket<Estimador> socket_initiator;
  tlm_utils::simple_target_socket<Estimador>    socket_target;

  // Constructor de Cotroler
  SC_CTOR(Estimador) : 
    socket_initiator("socket_initiator"),
    socket_target("socket_to_target")
  {
    //Se tienen las funciones TLM2
    socket_initiator.register_nb_transport_bw(this, &Estimador::nb_transport_bw);
    socket_target.register_nb_transport_fw(this, &Estimador::nb_transport_fw);

    //Se tiene las funciones recurrente
    SC_THREAD(thread_process_to_fw);
    SC_THREAD(thread_process_to_bw);  
    
    SC_THREAD(estimador_main);
    SC_THREAD(TB);
  }

  //==============================================================================================
  //                                   FUNCIONES DE INICIADOR
  //==============================================================================================

  //----------------------------------------------------------------------------------------------
  // Thread Iniciador
  void thread_process_to_bw(){
    
    tlm::tlm_generic_payload trans;
    sc_time delay = sc_time(DELAY_IP, SC_NS);

    ID_extension* id_extension = new ID_extension; //Se crea un ID con la clase anterior
    trans.set_extension( id_extension );

    for (int i = 0; i<0xF000000; i++){
      id_extension->transaction_id++;
    }

    while(Exe)
    {
      wait(do_initiator_t.default_event());

      tlm::tlm_phase phase = tlm::BEGIN_REQ;
      tlm::tlm_command cmd = static_cast<tlm::tlm_command>(comando_Initiator); //comando = (0 read, 1 write)
      
      //Parametros de trans
      trans.set_command( cmd );   
      trans.set_address( address_Initiator );   
      trans.set_data_ptr( reinterpret_cast<unsigned char*>(&data_Initiator) ); //Este es un puntero
      trans.set_data_length( 4 );   
      trans.set_byte_enable_ptr( 0 );

      tlm::tlm_sync_enum status;
      cout  << "0 - "<< name() << " BEGIN_REQ  SENT    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      
      wait(delay);
      status = socket_initiator->nb_transport_fw(trans, phase, delay );  // Non-blocking transport call   
      wait(delay);

      // Checkea el status de la transaccion   
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
                << hex << data_Initiator << " at time " << sc_time_stamp() << ", delay = " << delay << endl;
          cout << endl;
          break;   
      }

      initiator_done_t.notify();
      id_extension->transaction_id++;
    }
  }
   
  //----------------------------------------------------------------------------------------------
   
  virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase,
                                              sc_time& delay ){
    
    tlm::tlm_command cmd    =  trans.get_command();   
    sc_dt::uint64    adr    =  trans.get_address();   
    int              data_p;       
    memcpy(&data_p, trans.get_data_ptr(), 4);

    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension ); 
    
    if (phase == tlm::BEGIN_RESP) 
    {                          
      // Initiator obliged to check response status   
      if (trans.is_response_error() )   
        SC_REPORT_ERROR("TLM2", "Response error from nb_transport");   

      cout << endl;
      cout  << "0 - " 
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

    else if (phase == tlm::END_RESP) {  
           
      cout  << "0 - "<< name() << " END_RESP   RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

      cout << endl;
      cout << endl;
      cout << "------------------------------------------------------------------------" << endl;
      cout << endl;
      cout << endl;

      initiator_done_Resp_t.notify();
      return tlm::TLM_COMPLETED;
    }

    else{
      initiator_done_Resp_t.notify();
      return tlm::TLM_COMPLETED;
    }
  }   
  

  //==============================================================================================
  //                                   FUNCIONES DE TARGET
  //==============================================================================================

  //----------------------------------------------------------------------------------------------
  // Thread Target
  void thread_process_to_fw()
  {    
    while (true)
    { 
      wait(do_target_t.default_event()); 

      trans_pending = trans_pending_queue.front();
      phase_pending = phase_pending_queue.front();
      delay_pending = delay_pending_queue.front();

      trans_pending_queue.pop();
      phase_pending_queue.pop();
      delay_pending_queue.pop();
  
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
      delay_pending= sc_time(DELAY_IP, SC_NS);

      //********************************************************************************
      // SE MODIFICA SEGUN LO QUE SE NECESITE CUANDO SE VA A HACER LECTURAS/ESCRITURAS
      //********************************************************************************
      address_Target = static_cast<sc_uint<32> >( adr & 0xFFFFFFFF);
      float var_test_float;

      //Se copia el contenido del puntero a un intiger
      memcpy(&data_aux_Target, ptr, 4);
      //Paso de punto fijo a punto flotante
      var_test_float = (float)data_aux_Target;
      var_test_float = var_test_float/pow(2,21);

      cout << "Estimador ********IP ******* addrs   : "   << hex << address_Target  << endl;
      cout << "Estimador ********IP ******* data    : "   << var_test_float  << endl;
      cout << "Estimador ********IP ******* data hex: "   << hex << data_aux_Target << endl;

      if(address_Target == Start_Addr){
        start = 1;
        calc_t.notify(calc_delay, SC_NS);
      }
      
      else if(address_Target == I_scale_factor_Addr){
        I_scale_factor_e = var_test_float;
      }

      else if(address_Target == V_scale_factor_Addr){
        V_scale_factor_e = var_test_float;
      }

      else if(address_Target == Ig_value_Addr){
        Ig_e = var_test_float;
      }

      else if(address_Target == Gamma11_Addr){
        GAMMA11_e = var_test_float;
      }
      
      else if(address_Target == Gamma12_Addr){
        GAMMA12_e = var_test_float;
      }

      else if(address_Target == Gamma21_Addr){
        GAMMA21_e = var_test_float;
      }

      else if(address_Target == Gamma22_Addr){
        GAMMA22_e = var_test_float;
      }

      else if(address_Target == Init_alpha_Addr){
        INIT_ALPHA_e = var_test_float;
      }

      else if(address_Target == Init_beta_Addr){
        INIT_BETA_e = var_test_float;
      }

      else if(address_Target == T_sampling_Addr){
        T_SAMPLING_e = var_test_float;
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
                                              sc_time& delay)
  {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address();
    unsigned char*   ptr = trans.get_data_ptr();   
    unsigned int     len = trans.get_data_length();   
    unsigned char*   byt = trans.get_byte_enable_ptr();   
    unsigned int     wid = trans.get_streaming_width();   

    //Se hace una copia del ID de la transaccion
    ID_extension* id_extension = new ID_extension;
    ID_extension* id_extension2 = new ID_extension;

    trans.get_extension(id_extension);
    *id_extension2 = *id_extension;
    
    //
    // PRIMERA FASE DE LA TRANSACCION
    //
    if(phase == tlm::BEGIN_REQ)
    {
      // Verifica errores
      if (byt != 0) 
      {
        cout << "ERROR 1" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
        trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
        return tlm::TLM_COMPLETED;
      }
      
      if (len > 4 || wid != 0)
      {
        cout << "ERROR 2" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;     
        trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
        return tlm::TLM_COMPLETED;
      }
      
      //==========================================================================
      // Se hace una copia de las transacciones para dar lugar a otras
      tlm::tlm_generic_payload* trans_aux = new tlm::tlm_generic_payload;
      int * data_ptr = new int;
      
      memcpy(data_ptr, ptr, 4);

      trans_aux->set_command( cmd );   
      trans_aux->set_address( adr );   
      
      if ( cmd == tlm::TLM_WRITE_COMMAND ){
        trans_aux->set_data_ptr( reinterpret_cast<unsigned char*>(data_ptr) );
      }

      else if ( cmd == tlm::TLM_READ_COMMAND ){
        trans_aux->set_data_ptr( ptr );
      }
      
      trans_aux->set_data_length( len );   
      trans_aux->set_byte_enable_ptr( byt );
      trans_aux->set_extension( id_extension2 );

      //==========================================================================
      // Se hace un push en los queue para dar espacio a otras transacciones
      trans_pending_queue.push(trans_aux);
      phase_pending_queue.push(phase);
      delay_pending_queue.push(delay);

      cout  << "1 - "   << name() << "    BEGIN_REQ  RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;            
      do_target_t.notify(0, SC_NS);
      return tlm::TLM_ACCEPTED;
    }


    //
    // SEGUNDA FASE DE LA TRANSACCION
    //
    else if(phase == tlm::END_REQ)
    {
      cout  << "1 - "   << name() << "    END_REQ    RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      return tlm::TLM_COMPLETED;
    }

    else
    {
      return tlm::TLM_COMPLETED;
    }
  };
  
  //==============================================================================================
  //                                   FUNCIONES TB
  //==============================================================================================

  float InputVoltage(float t){
    return (V_cte + (0.3 * V_cte * sin(2 * M_PI * 1000 * t)));
  }

  float InputCurrent(float t){
    return (Lambda - exp( alpha * InputVoltage(t) + b));
  }

  uint16_t to_fixed_16(float a){
    a=a*pow(2,16);
    int b = (int)a;
    return INT2U16(b);
  }

  uint32_t to_fixed_32(float a){
    a=a*pow(2,21);
    int b = (int)a;
    return INT2U32(b);
  }

  //Datos listos
  void process_sample() {
    calc_t.notify(calc_delay, SC_NS);
  }

  void estimador_main(){

    while(true){

      wait(calc_t);

      if(start){
        wait(120, SC_NS);
        cout << "Estimador *******************************" << endl;
        cout << "Estimador        === SUMMERY ===" << endl;
        cout << "Estimador I_scale_factor_e: " << I_scale_factor_e << endl;
        cout << "Estimador V_scale_factor_e: " << V_scale_factor_e << endl;
        cout << "Estimador Ig_e            : " << Ig_e << endl;
        cout << "Estimador GAMMA11_e       : " << GAMMA11_e << endl;
        cout << "Estimador GAMMA12_e       : " << GAMMA12_e << endl;
        cout << "Estimador GAMMA21_e       : " << GAMMA21_e << endl;
        cout << "Estimador GAMMA22_e       : " << GAMMA22_e << endl;
        cout << "Estimador INIT_ALPHA_e    : " << INIT_ALPHA_e << endl;
        cout << "Estimador INIT_BETA_e     : " << INIT_BETA_e << endl;
        cout << "Estimador T_SAMPLING_e    : " << T_SAMPLING_e << endl;
        cout << "Estimador *******************************" << endl;
        
        init_cond_1 = INIT_ALPHA_e;
        init_cond_2 = INIT_BETA_e;
        start = 0;
      };

      I = adc_i / pow(2,16);
      V = adc_v / pow(2,16);

      I *= I_scale_factor_e;
      V *= V_scale_factor_e;
      y_log = log(Ig_e - I);
      p1=((GAMMA11_e*V+GAMMA12_e)*(y_log-(V*init_cond_1)-init_cond_2))*T_SAMPLING_e+init_cond_1;
      p2=((GAMMA21_e*V+GAMMA22_e)*(y_log-(V*init_cond_1)-init_cond_2))*T_SAMPLING_e+init_cond_2;
      init_cond_1=p1;
      init_cond_2=p2;

      //=================================

      param_1 = to_fixed_32(p1);
      comando_Initiator = 1;
      data_Initiator    = param_1;
      address_Initiator   = 0x43C00010;
      address_Initiator   = address_Initiator | 0xEB00000000;

      cout << "Estimador " << endl;
      cout << "Estimador IP  write data p1    : " << p1 << endl;
      cout << "Estimador IP  write data hex p1: " << hex << data_Initiator  << endl;
      cout << "Estimador IP  write addr p1    : " << hex << address_Initiator << endl;
      
      do_initiator_t.notify(0,SC_NS);
      wait(initiator_done_t);
      wait(80, SC_NS);

      //=================================

      param_2 = to_fixed_32(p2);
      comando_Initiator = 1;
      data_Initiator    = param_2;
      address_Initiator   = 0x43C00014;
      address_Initiator   = address_Initiator | 0xEB00000000;

      cout << "Estimador " << endl;
      cout << "Estimador IP  write data p2    : " << p2 << endl;
      cout << "Estimador IP  write data hex p2: " << hex << data_Initiator  << endl;
      cout << "Estimador IP  write addr p2    : " << hex << address_Initiator << endl;

      do_initiator_t.notify(0,SC_NS);
      wait(initiator_done_t);
      wait(80, SC_NS);

      //=================================

      volt = to_fixed_32(V);
      comando_Initiator = 1;
      data_Initiator    = volt;
      address_Initiator   = 0x43C00018;
      address_Initiator   = address_Initiator | 0xEB00000000;

      cout << "Estimador " << endl;
      cout << "Estimador IP  write data V    : " << V << endl;
      cout << "Estimador IP  write data hex V: " << hex << data_Initiator  << endl;
      cout << "Estimador IP  write addr V    : " << hex << address_Initiator << endl;

      do_initiator_t.notify(0,SC_NS);
      wait(initiator_done_t);
      wait(80, SC_NS);

      //=================================

      current = to_fixed_32(I);
      comando_Initiator = 1;
      data_Initiator    = current;
      address_Initiator  = 0x43C0001C;
      address_Initiator  = address_Initiator | 0xEB00000000;

      cout << "Estimador " << endl;
      cout << "Estimador IP  write data I    : " << I << endl;
      cout << "Estimador IP  write data hex I: " << hex << data_Initiator  << endl;
      cout << "Estimador IP  write addr I    : " << hex << address_Initiator << endl;

      do_initiator_t.notify(0,SC_NS);
      wait(initiator_done_t);
      wait(80, SC_NS);
      done_IP.notify();

      cout << "Estimador1 time out " << "@" << sc_time_stamp()<< endl;
    }
  }

  void TB(){

    cout << endl;
    cout << endl;
    
    // Open VCD file
    sc_trace_file *wf = sc_create_vcd_trace_file("estimador");
    wf->set_time_unit(1, SC_NS);

    //open CSV file
    std::ofstream file_Signals;
    file_Signals.open ("SIGNALS.CSV");
    std::ofstream file_Params;
    file_Params.open ("PARAMS.CSV");
    
    // Dump the desired signals
    sc_trace(wf, adc_v, "adc_v");
    sc_trace(wf, adc_i, "adc_i");
    sc_trace(wf, start, "start");
    sc_trace(wf, p1, "param_1");
    sc_trace(wf, p2, "param_2");
    sc_trace(wf, volt, "volt");
    sc_trace(wf, current, "current");

    wait(done_IP);
    
    for(int i = 0; i < n_samples; i++){  

      start = 0;
      V_TB = InputVoltage(t)/22;
      I_TB = InputCurrent(t)/5;

      adc_v = to_fixed_16(V_TB);
      adc_i = to_fixed_16(I_TB);

      file_Signals << t <<","<< I_TB << ","<< V_TB << endl;
      file_Params << t << ","<< param_1 << ","<< param_2 << endl;
      t = t + step;

      cout << "Estimador1 " << "@" << sc_time_stamp()<< endl;
      process_sample();
      cout << "Estimador1 " << dec << "iter = "<<i<<endl;
      wait(done_IP);
    }
    
    cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
    
    //Close files
    sc_close_vcd_trace_file(wf);
    file_Signals.close();
    file_Params.close(); 

    Exe = false;
  }
  
  //Variables TB
  bool Exe = true;
  
  //Variables de puerto Target
  sc_event_queue  do_target_t; 
  std::queue<tlm::tlm_generic_payload*> trans_pending_queue;   
  std::queue<tlm::tlm_phase>            phase_pending_queue;   
  std::queue<sc_time>                   delay_pending_queue;

  tlm::tlm_generic_payload* trans_pending;   
  tlm::tlm_phase phase_pending;   
  sc_time delay_pending;
  
  sc_event target_done_t;
  int data_aux_Target;
  unsigned char *data_Target;
  sc_uint<32> address_Target;

  //Variables de puerto Iniciador  
  sc_event_queue do_initiator_t;
  sc_event initiator_done_t, initiator_done_Resp_t;
  int data_Initiator;
  long int address_Initiator;
  bool comando_Initiator;

  //Variables internas
  float I_scale_factor_e, V_scale_factor_e, Ig_e, GAMMA11_e, GAMMA12_e, GAMMA21_e, GAMMA22_e, INIT_ALPHA_e, INIT_BETA_e, T_SAMPLING_e;

  sc_uint<16> adc_v; // vector data from XADC
  sc_uint<16> adc_i; // vector data from XADC

  bool  start;         // Active high, ready signal from estimador
  sc_uint<32> param_1; // 32 bit vector output of the estimador
  sc_uint<32> param_2; // 32 bit vector output of the estimador
  sc_uint<32> volt;
  sc_uint<32> current;
  
  sc_event calc_t, done_IP;

  float init_cond_1, init_cond_2; 
  float p1, p2, p1_aux, p2_aux, y_log, I, V;

  float Lambda = 3.99;                     	//Short-Circuit current
  float Psi = 5.1387085e-6;                	//Is current (saturation)
  float alpha = 0.625;                 			//Thermal voltage relation
  float V_oc = 1/alpha*(log(Lambda/Psi));   //Open circuit voltage
  float V_mpp = 17.4;                  			//Maximum power point voltage
  float I_mpp = 3.75;                  			//Maximum power point current
  float P_mpp = 65.25;                 			//Maximum power 
  float y = log(Lambda);              			//Short-Circuit logarithm
  float b = log(Psi);                 			//Is current logarithm
  float V_cte = 16.69;

  float t = 0;
  float segundos=3;
  float sample_rate=1e6;
  float step=1/sample_rate;
  float n_samples=segundos*sample_rate;
  float V_TB, I_TB;
};


#endif