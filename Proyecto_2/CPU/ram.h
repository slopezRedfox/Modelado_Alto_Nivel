#ifndef RAM_H
#define RAM_H

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include <queue>

//Constantes de memoria cache
#define SIZE 0x4FFFFFFF
#define wr_delay_ram 15
#define rd_delay_ram 10

//----------------------------------------------------------------------------------------------------------------------
//Modulo de Ram

struct Ram: sc_module {   
  
  tlm_utils::simple_initiator_socket<Ram> socket_initiator;
  tlm_utils::simple_target_socket<Ram> socket_target;
  const sc_time LATENCY;   
  
  //Se declara el constructor y se establece la latencia a 10ns
  SC_CTOR(Ram) : 
    socket_target("socket_target"), 
    socket_initiator("socket_initiator"),
    LATENCY(10, SC_NS)
  {
    // Register callbacks for incoming interface method calls
    socket_target.register_nb_transport_fw(this, &Ram::nb_transport_fw);
    socket_initiator.register_nb_transport_bw(this, &Ram::nb_transport_bw);

    //Se establece una funcion recurente
    SC_THREAD(thread_process_to_bw);
    SC_THREAD(thread_process_to_fw);
    SC_THREAD(wr);
    SC_THREAD(rd);
  }

    //==============================================================================================
    //                                   FUNCIONES DE TARGET
    //==============================================================================================

    // Thread Iniciador
    //----------------------------------------------------------------------------------------------
    void thread_process_to_fw(){
        
        while (true) 
        {
            wait(e1); 
        
            tlm::tlm_phase phase;
            ID_extension* id_extension = new ID_extension;
            trans_pending->get_extension( id_extension ); 
            
            //Parametros del paquete de transaccion
            tlm::tlm_command cmd = trans_pending->get_command();
            sc_dt::uint64    adr = trans_pending->get_address();
            unsigned char*   ptr = trans_pending->get_data_ptr();   
            unsigned int     len = trans_pending->get_data_length();   
            unsigned char*   byt = trans_pending->get_byte_enable_ptr();   
            unsigned int     wid = trans_pending->get_streaming_width();   

            //Al igual que en el punto anterior se revisa que todos los datos de la transaccione esten bien
            if ((adr & 0xFFFFFFFF) >= sc_dt::uint64(SIZE) || byt != 0 || wid != 0 || len > 4)   
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
                data = ptr;
                address = static_cast<sc_uint<32> >( adr & 0xFFFFFFFF);
                read();
                wait(done_t);
            }

            else if ( cmd == tlm::TLM_WRITE_COMMAND ){
                data = ptr;
                address = static_cast<sc_uint<32> >( adr & 0xFFFFFFFF);
                write();
                wait(done_t);
            }
            //********************************************************************************
            //********************************************************************************
            
            tlm::tlm_sync_enum status;
            phase  = tlm::BEGIN_RESP; 
            
            wait( sc_time(10, SC_NS) );
            cout  << "1 - "   << name() << "    BEGIN_RESP SENT    " << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
            
            status = socket_target->nb_transport_bw( *trans_pending, phase, delay_pending );   

            switch (status)     
                case tlm::TLM_ACCEPTED:   
                
                wait( sc_time(10, SC_NS) );          
                cout  << "1 - "   << name() << "    END_RESP   SENT    " << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
                phase = tlm::END_RESP;
                
                socket_target->nb_transport_bw( *trans_pending, phase, delay_pending );  // Non-blocking transport call
            

            //********************************************************************************
            //                     SOLO SE USA PARA PRUEBAS DE LA RAM 
            //********************************************************************************
            cout << endl;
            cout << "Memoria RAM" << endl;
            cout << endl;

            double add;
            if (address < 8)
            {
                add = 8;
            }
            else if (address > SIZE - 8){
                add = SIZE - 8;
            }
            else{
                add = address;
            }
            
            cout << endl;
            cout << "Address: " << address << endl;
            cout << endl;

            for (int j = add - 8; j < add + 8; j++){
                cout << "Cel Num #" << hex << j << " | data: " << hex << mem[j] << endl;
            }
            cout << endl;
            //********************************************************************************
            //********************************************************************************
        }   
    }   
    
    //----------------------------------------------------------------------------------------------

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
                                                tlm::tlm_phase& phase, 
                                                sc_time& delay){
        

        sc_dt::uint64    adr = trans.get_address();
        unsigned int     len = trans.get_data_length();
        unsigned char*   byt = trans.get_byte_enable_ptr();
        unsigned int     wid = trans.get_streaming_width();

        //Se le coloca un ID a cada transaccion
        ID_extension* id_extension = new ID_extension;
        trans.get_extension( id_extension ); //Se extrae el ID de la transaccion

        //
        // PRIMERA FASE DE LA TRANSACCION
        //
        if(phase == tlm::BEGIN_REQ){
            // Primero se pregunta si el dato a transmitir esta abilitado
            if (byt != 0) {
                //
                trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
                return tlm::TLM_COMPLETED;
            }
            
            // Luego se pregunta si el tamaño del paquete es el adecuado
            if (len > 4 || wid != 0) {
                trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
                return tlm::TLM_COMPLETED;
            }

            // Se pasa los datos a las variables globales de la estructura
            // Que estan declaradas al final de la misma, para poder ser
            // trabajados en la funcion thread
            trans_pending = &trans;
            phase_pending = phase;
            delay_pending = delay;

            wait(delay);
            cout  << "1 - "   << name() << "    BEGIN_REQ  RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;            

            // Se hace un notify de e1 para permitir a la funcion thread continuar
            e1.notify();

            //Se envia que la transaccion ha sido aceptada
            return tlm::TLM_ACCEPTED;
        }


        //
        // SEGUNDA FASE DE LA TRANSACCION
        //
        else if(phase == tlm::END_REQ){

            wait(delay);
            cout  << "1 - "   << name() << "    END_REQ    RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
            
            //Se imprime que se termino el request
            //Devuelve la confirmacion de que se TERMONO la transaccion
            return tlm::TLM_COMPLETED;
        }

        else{
            return tlm::TLM_COMPLETED;
        }
    };
  
  
    //==============================================================================================
    //                                   FUNCIONES DE INICIADOR
    //==============================================================================================

    //----------------------------------------------------------------------------------------------
    // Thread Iniciador
    void thread_process_to_bw(){
        
        tlm::tlm_generic_payload trans;
        sc_time delay = sc_time(10, SC_NS);

        while(false){
            wait(initiator_t);                

            trans.set_extension( id_extension_initiator );

            //PHASE == BEGIN_REQ
            tlm::tlm_phase phase = tlm::BEGIN_REQ;
            tlm::tlm_command cmd = static_cast<tlm::tlm_command>(comando_Initiator); //comando_Initiator = (0 read, 1 write)
            
            //Parametros de trans
            trans.set_command( cmd );   
            trans.set_address( address_Initiator );   
            trans.set_data_ptr( reinterpret_cast<unsigned char*>(&data_Initiator) ); //Este es un puntero
            trans.set_data_length( 4 );   
            trans.set_byte_enable_ptr( 0 );

            tlm::tlm_sync_enum status;

            wait( sc_time(10, SC_NS) );
            cout  << "0 - "<< name() << " BEGIN_REQ  SENT    " << " TRANS ID " << id_extension_initiator->transaction_id << " at time " << sc_time_stamp() << endl;

            status = socket_initiator->nb_transport_fw(trans, phase, delay );  // Non-blocking transport call   
            
            // Checkea el status de la transaccion   
            switch (status)
            {
                case tlm::TLM_ACCEPTED:   
                
                    wait( sc_time(10, SC_NS) );
                    cout  << "0 - "<< name() << " END_REQ    SENT    " << " TRANS ID " << id_extension_initiator->transaction_id << " at time " << sc_time_stamp() << endl;
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
            
            //Delay between RD/WR request
            wait(100, SC_NS);    
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

            //Delay para BEGIN_RESP
            wait(delay);
            cout  << "0 - "<< name () << " BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension_initiator->transaction_id << " at time " << sc_time_stamp() << endl;
            return tlm::TLM_ACCEPTED;
        } 

        else if (phase == tlm::END_RESP) 
        {
            //Delay for END_RESP
            wait(delay);
            cout  << "0 - "<< name() << " END_RESP   RECEIVED" << " TRANS ID " << id_extension_initiator->transaction_id << " at time " << sc_time_stamp() << endl;
            initiator_done_t.notify();
            return tlm::TLM_COMPLETED;
        }

        else
        {
            initiator_done_t.notify();
            return tlm::TLM_COMPLETED;
        }
    }



  
  //==============================================================================================

    void write() {
        wr_t.notify(wr_delay_ram, SC_NS);
    }

    void read() {
        rd_t.notify(rd_delay_ram, SC_NS);  
    }

    //Escritura
    void wr() 
    {
        while(true) 
        {
            //Espera tiempo de escritura
            wait(wr_t);
            memcpy(&data_aux, data, 4);
            
            mem[address] = data_aux;
            done_t.notify();
        }
    }

    //Lectura
    void rd() 
    {
        while(true) 
        {
            //Espera tiempo de lectura
            wait(rd_t);
            data_aux = mem[address];

            memcpy(data, &data_aux, 4);
            done_t.notify();
        }
    }
    
    //Parametros nuevos
    sc_event  e1; 
    tlm::tlm_generic_payload* trans_pending;   
    tlm::tlm_phase phase_pending;   
    sc_time delay_pending;
    sc_event done_t;
    int data_aux;
    
    int mem[SIZE];

    mem[0x43c00010] = 0xFFFFFFFF;
    mem[0x43c00014] = 0xAAAAAAAA;
    mem[0x43c00018] = 0xBBBBBBBB;
    mem[0x43c0001c] = 0xCCCCCCCC;

    sc_event wr_t, rd_t;
    sc_uint<32> address;
    unsigned char* data;

    //Variables de puerto Iniciador  
    int data_Initiator;  
    long int address_Initiator;
    bool comando_Initiator;
    sc_event initiator_t, initiator_done_t;
    ID_extension* id_extension_initiator = new ID_extension; //Se crea un ID con la clase anterior
};

#endif