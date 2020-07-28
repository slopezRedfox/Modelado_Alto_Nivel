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

#define INT2U32(x) *(uint32_t*)&x
#define INT2U16(x) *(uint16_t*)&x

# define M_PI           3.14159265358979323846  /* pi */

#define DELAY_IP 2

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

      status = socket_initiator->nb_transport_fw(trans, phase, delay );  // Non-blocking transport call   
      wait(delay);
      wait(auxC);

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
                << hex << data << " at time " << sc_time_stamp() << ", delay = " << delay << endl;
          cout << endl;
          break;   
      }

      //Delay between RD/WR request
      wait(10, SC_NS);   
      
      id_extension->transaction_id++;
      done_tC.notify();
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
    
    if (phase == tlm::BEGIN_RESP) {
                              
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

      //Delay para BEGIN_RESP
      cout  << "0 - "<< name () << " BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      auxC.notify();
      return tlm::TLM_ACCEPTED;
    } 

    else if (phase == tlm::END_RESP) {  
           
      //Delay for END_RESP
      cout  << "0 - "<< name() << " END_RESP   RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
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
        delay_pending= sc_time(DELAY_IP, SC_NS);

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
      if(phase == tlm::BEGIN_REQ)
      {    
          //Se revisa que todos los datos de la transaccione esten bien
          if (byt != 0 || wid != 0 || len > 4){  
              SC_REPORT_ERROR("TLM2", "Target does not support given generic payload transaction");   
          }
          trans_pending = &trans;
          phase_pending = phase;
          delay_pending = delay;

          cout  << "1 - "   << name() << "    BEGIN_REQ  RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;            

          // Se hace un notify de e1 para permitir a la funcion thread ejecutar
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

      else{
        return tlm::TLM_COMPLETED;
      }
  }


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
        cout << "Hola mundo" << endl;
        init_cond_1 = INIT_ALPHA;
        init_cond_2 = INIT_BETA;
      };

      I = adc_i / pow(2,16);
      V = adc_v / pow(2,16);

      I *= I_scale_factor;
      V *= V_scale_factor;
      y_log = log(Ig - I);
      p1=((GAMMA11*V+GAMMA12)*(y_log-(V*init_cond_1)-init_cond_2))*T_SAMPLING+init_cond_1;
      p2=((GAMMA21*V+GAMMA22)*(y_log-(V*init_cond_1)-init_cond_2))*T_SAMPLING+init_cond_2;
      init_cond_1=p1;
      init_cond_2=p2;

      cout<<"param_1 = "<< p1 <<"   param_2 = "<< p2 <<endl<<endl;

      param_1 = to_fixed_32(p1);
      cout << "Estimador p1: " << p1 << endl;
      comando = 1;
      data    = param_1;
      addrs  = 0x43C00010;
      addrs  = addrs | 0xCB00000000;
      do_t.notify(0,SC_NS);
      wait(done_tC);

      param_2 = to_fixed_32(p2);
      cout << "Estimador p2: " << p2 << endl;
      comando = 1;
      data    = param_2;
      addrs  = 0x43C00014;
      addrs  = addrs | 0xCB00000000;
      do_t.notify(0,SC_NS);
      wait(done_tC);

      volt = to_fixed_32(V);
      cout << "Estimador V: " << V << endl;
      comando = 1;
      data    = volt;
      addrs  = 0x43C00018;
      addrs  = addrs | 0xCB00000000;
      do_t.notify(0,SC_NS);
      wait(done_tC);

      current = to_fixed_32(I);
      cout << "Estimador I: " << I << endl;
      comando = 1;
      data    = current;
      addrs  = 0x43C0001C;
      addrs  = addrs | 0xCB00000000;
      do_t.notify(0,SC_NS);
      wait(done_tC);
      
      done_tt.notify();
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
    sc_trace(wf, param_1, "param_1");
    sc_trace(wf, param_2, "param_2");
    sc_trace(wf, volt, "volt");
    sc_trace(wf, current, "current");

    //Inicio del test
    
    cout << "@" << sc_time_stamp()<< endl;
    start = 1;
    adc_v = 0;
    adc_i = 0;
    process_sample();
    wait(done_tt);
    printf("first\n");

    //cout << "step= " << step << endl;
    
    for(int i = 0; i < 40; i++){  

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
      wait(100,SC_NS);
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
  
  //Variables de Iniciador
  sc_event_queue do_t;
  sc_event  done_tC, auxC;
  int data;  
  long int addrs;
  bool comando;  

  //Variables de puerto Target
  sc_event  target_t; 
  tlm::tlm_generic_payload* trans_pending;   
  tlm::tlm_phase phase_pending;   
  sc_time delay_pending;


  //Variables del IP
  //-----------IP Ports----------------------------------
  sc_uint<16> adc_v; // vector data from XADC
  sc_uint<16> adc_i; // vector data from XADC

  bool  start;         // Active high, ready signal from estimador
  sc_uint<32> param_1; // 32 bit vector output of the estimador
  sc_uint<32> param_2; // 32 bit vector output of the estimador
  sc_uint<32> volt;
  sc_uint<32> current;
  
  //-----------Internal variables------------------------
  sc_event calc_t, done_tt;

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