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
#define wr_delay_ram 4
#define rd_delay_ram 2

#define DELAY_RAM 5

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
    LATENCY(DELAY_RAM, SC_NS)
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
            wait(e1.default_event()); 

            trans_pending = trans_pending_queue.front();
            phase_pending = phase_pending_queue.front();
            delay_pending = delay_pending_queue.front();

            trans_pending_queue.pop();
            phase_pending_queue.pop();
            delay_pending_queue.pop();

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
            delay_pending= sc_time(DELAY_RAM, SC_NS);

            //********************************************************************************
            // SE MODIFICA SEGUN LO QUE SE NECESITE CUANDO SE VA A HACER LECTURAS/ESCRITURAS
            //********************************************************************************
            if ( cmd == tlm::TLM_READ_COMMAND )
            {
                data = ptr;
                address = static_cast<sc_uint<32> >( adr & 0xFFFFFFFF);
                read();
                wait(done_t);

                memcpy(&data_aux, ptr, 4);
                cout << "Estimador " << endl;
                cout << "Estimador ram read data    : " << (float)(data_aux)/pow(2,21) << " TRANS ID " << id_extension->transaction_id << endl;
                cout << "Estimador ram read data hex: " << data_aux                    << " TRANS ID " << id_extension->transaction_id << endl;
                cout << "Estimador ram read addr    : " << address                     << " TRANS ID " << id_extension->transaction_id << endl;
            }

            else if ( cmd == tlm::TLM_WRITE_COMMAND ){
                memcpy(&data_aux, ptr, 4);
                address = static_cast<sc_uint<32> >( adr & 0xFFFFFFFF);

                cout << "Estimador " << endl;
                cout << "Estimador ram write data    : " << (float)(data_aux)/pow(2,21) << " TRANS ID " << id_extension->transaction_id << endl;
                cout << "Estimador ram write data hex: " << data_aux                    << " TRANS ID " << id_extension->transaction_id << endl;
                cout << "Estimador ram write addr    : " << address                     << " TRANS ID " << id_extension->transaction_id << endl;

                write();
                wait(done_t);
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
                
                    cout  << " 1 - "   << name() << "    END_RESP   SENT    " << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
                    phase = tlm::END_RESP;
                    
                    socket_target->nb_transport_bw( *trans_pending, phase, delay_pending );  // Non-blocking transport call
                

            /*/********************************************************************************
            //                     SOLO SE USA PARA PRUEBAS DE LA RAM 
            //********************************************************************************
            cout << endl;
            cout << " TRANS ID " << id_extension->transaction_id << " Memoria RAM" << endl;
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
            
            cout << "Estimador CPU *******************************************************" << endl;
            cout << "Estimador CPU Memoria RAM" << endl;
            for (int j = add - 8; j < add + 8; j++){
                cout << "Estimador CPU TRANS ID " << id_extension->transaction_id << " Cel Num #" << hex << j << " | data: " << hex << mem[j] << endl;
            }
            cout << "Estimador CPU *******************************************************" << endl;

            cout << endl;
            //********************************************************************************
            //*********************************************************************************/
        }   
    }   
    
    //----------------------------------------------------------------------------------------------

    virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
                                                tlm::tlm_phase& phase, 
                                                sc_time& delay){
        

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
            e1.notify(0, SC_NS);

            //Se envia que la transaccion ha sido aceptada
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
    //                                   FUNCIONES DE INICIADOR
    //==============================================================================================

    //----------------------------------------------------------------------------------------------
    // Thread Iniciador
    void thread_process_to_bw(){
        
        tlm::tlm_generic_payload trans;
        sc_time delay = sc_time(DELAY_RAM, SC_NS);

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

            wait( delay );
            cout  << "0 - "<< name() << " BEGIN_REQ  SENT    " << " TRANS ID " << id_extension_initiator->transaction_id << " at time " << sc_time_stamp() << endl;

            status = socket_initiator->nb_transport_fw(trans, phase, delay );  // Non-blocking transport call   
            wait( delay );

            switch (status)
            {
                case tlm::TLM_ACCEPTED:   
                
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

            cout  << "0 - "<< name () << " BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension_initiator->transaction_id << " at time " << sc_time_stamp() << endl;
            return tlm::TLM_ACCEPTED;
        } 

        else if (phase == tlm::END_RESP) 
        {
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
    //==============================================================================================

    // ****************
    // INTERNALS
    // ****************

    void write()
    {
        wr_t.notify(wr_delay_ram, SC_NS);
    }

    void read()
    {
        rd_t.notify(rd_delay_ram, SC_NS);  
    }

    //Escritura
    void wr() 
    {
        while(true) 
        {
            //Espera tiempo de escritura
            wait(wr_t);            
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
    
    //Variables de puerto Target
    sc_event_queue  e1; 
    std::queue<tlm::tlm_generic_payload*> trans_pending_queue;   
    std::queue<tlm::tlm_phase>            phase_pending_queue;   
    std::queue<sc_time>                   delay_pending_queue;

    tlm::tlm_generic_payload* trans_pending;   
    tlm::tlm_phase phase_pending;   
    sc_time delay_pending;
    
    sc_event done_t;
    int data_aux;
    unsigned char* data;
    sc_uint<32> address;

    //Variables de puerto Iniciador  
    int data_Initiator;  
    long int address_Initiator;
    bool comando_Initiator;
    sc_event initiator_t, initiator_done_t;
    ID_extension* id_extension_initiator = new ID_extension; //Se crea un ID con la clase anterior

    //Variables internas
    int mem[SIZE];
    sc_event wr_t, rd_t;
};

#endif