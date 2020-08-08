#ifndef ROUTER_H
#define ROUTER_H

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include <queue>

#define DELAY_ROUTER 1

// *********************************************
// Generic payload blocking transport router
// *********************************************

struct Router: sc_module{

    tlm_utils::simple_initiator_socket<Router> socket_initiator_A;
    tlm_utils::simple_target_socket<Router>    socket_target_A;

    tlm_utils::simple_initiator_socket<Router> socket_initiator_B;
    tlm_utils::simple_target_socket<Router>    socket_target_B;

    tlm_utils::simple_initiator_socket<Router> socket_initiator_C;
    tlm_utils::simple_target_socket<Router>    socket_target_C;

    SC_CTOR(Router):
        socket_initiator_A("socket_initiator_A"),
        socket_target_A("socket_target_A"),

        socket_initiator_B("socket_initiator_B"),
        socket_target_B("socket_target_B"),

        socket_initiator_C("socket_initiator_C"),
        socket_target_C("socket_target_C")
    {

        socket_initiator_A.register_nb_transport_bw(this, &Router::nb_transport_bw_A);
        socket_target_A.register_nb_transport_fw(this, &Router::nb_transport_fw_A);

        socket_initiator_B.register_nb_transport_bw(this, &Router::nb_transport_bw_B);
        socket_target_B.register_nb_transport_fw(this, &Router::nb_transport_fw_B);

        socket_initiator_C.register_nb_transport_bw(this, &Router::nb_transport_bw_C);
        socket_target_C.register_nb_transport_fw(this, &Router::nb_transport_fw_C);

        SC_THREAD(thread_process_to_bw_A);
        SC_THREAD(thread_process_to_fw_A);

        SC_THREAD(thread_process_to_bw_B);
        SC_THREAD(thread_process_to_fw_B);

        SC_THREAD(thread_process_to_bw_C);
        SC_THREAD(thread_process_to_fw_C);
    }


    //==============================================================================================
    //                                   FUNCIONES DE TARGET
    //==============================================================================================

    // Desde Target #A
    virtual tlm::tlm_sync_enum nb_transport_bw_A( tlm::tlm_generic_payload& trans,
                                                    tlm::tlm_phase& phase,
                                                    sc_time& delay )
    {
        ID_extension* id_extension = new ID_extension;
        trans.get_extension( id_extension ); 

        unsigned int target_nr = decode_address_T( trans.get_address());
        unsigned int initiator_nr = decode_address_I( trans.get_address());
        cout << "tlm_b A Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Target #" << hex << target_nr << "    -> Router)" << endl;

        if(initiator_nr == 0xB)
        {
            trans_bw_B.push(&trans);
            phase_bw_B.push(phase);
            delay_bw_B.push(delay);
            bw_t_B.notify(0,SC_NS);
            wait(bw_t_end_A.default_event());
            
            cout << "tlm_b A Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            Aux_bw_A = status_bw_B.front();
            status_bw_B.pop();
            return Aux_bw_A;
        }
        
        else if (initiator_nr == 0xC)
        {
            trans_bw_C.push(&trans);
            phase_bw_C.push(phase);
            delay_bw_C.push(delay);
            bw_t_C.notify(0,SC_NS);
            wait(bw_t_end_A.default_event());
            
            cout << "tlm_b A Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            Aux_bw_A = status_bw_C.front();
            status_bw_C.pop();
            return Aux_bw_A;
        }
        
        else
        {
            cout << "tlm_b A ERROR" << endl;
            return tlm::TLM_COMPLETED;
        }
    }

    // Desde Target #B
    virtual tlm::tlm_sync_enum nb_transport_bw_B( tlm::tlm_generic_payload& trans,
                                                    tlm::tlm_phase& phase,
                                                    sc_time& delay )
    {
        ID_extension* id_extension = new ID_extension;
        trans.get_extension( id_extension ); 

        unsigned int target_nr = decode_address_T( trans.get_address());
        unsigned int initiator_nr = decode_address_I( trans.get_address());
        cout << "tlm_b B Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Target #" << hex << target_nr << "    -> Router)" << endl;

        tlm::tlm_generic_payload* trans_pending;   

        if(initiator_nr == 0xA)
        {
            trans_bw_A.push(&trans);
            phase_bw_A.push(phase);
            delay_bw_A.push(delay);
            bw_t_A.notify(0,SC_NS);
            wait(bw_t_end_B.default_event());
            
            cout << "tlm_b B Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            Aux_bw_B = status_bw_A.front();
            status_bw_A.pop();
            return Aux_bw_B;
        }
        
        else if(initiator_nr == 0xC)
        {
            trans_bw_C.push(&trans);
            phase_bw_C.push(phase);
            delay_bw_C.push(delay);
            bw_t_C.notify(0,SC_NS);
            wait(bw_t_end_C.default_event());
            
            cout << "tlm_b B Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            Aux_bw_B = status_bw_C.front();
            status_bw_C.pop();
            return Aux_bw_B;
        }
        else
        {
            cout << "tlm_b B ERROR" << endl;
            return tlm::TLM_COMPLETED;
        }
    }

    // Desde Target #C
    virtual tlm::tlm_sync_enum nb_transport_bw_C( tlm::tlm_generic_payload& trans,
                                                    tlm::tlm_phase& phase,
                                                    sc_time& delay )
    {
        ID_extension* id_extension = new ID_extension;
        trans.get_extension( id_extension ); 

        unsigned int target_nr = decode_address_T( trans.get_address());
        unsigned int initiator_nr = decode_address_I( trans.get_address());
        cout << "tlm_b C Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Target #" << hex << target_nr << "    -> Router)" << endl;

        tlm::tlm_generic_payload* trans_pending;   

        if(initiator_nr == 0xA)
        {
            trans_bw_A.push(&trans);
            phase_bw_A.push(phase);
            delay_bw_A.push(delay);
            bw_t_A.notify(0,SC_NS);
            wait(bw_t_end_C.default_event());
            
            cout << "tlm_b C Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            Aux_bw_C = status_bw_A.front();
            status_bw_A.pop();
            return Aux_bw_C;
        }
        
        else if(initiator_nr == 0xB)
        {
            trans_bw_B.push(&trans);
            phase_bw_B.push(phase);
            delay_bw_B.push(delay);
            bw_t_B.notify(0,SC_NS);
            wait(bw_t_end_C.default_event());
            
            cout << "tlm_b C Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            Aux_bw_C = status_bw_B.front();
            status_bw_B.pop();
            return Aux_bw_C;
        }        
        else
        {
            cout << "tlm_b C ERROR" << endl;
            return tlm::TLM_COMPLETED;
        }
    }

    //----------------------------------------------------------------------------------------------

    // Respuesta a Iniciador A desde Target
    void thread_process_to_bw_A(){
        while (true)
        {  
            wait(bw_t_A.default_event());

            tlm::tlm_generic_payload* trans = trans_bw_A.front();
            tlm::tlm_phase            phase = phase_bw_A.front();
            sc_time                   delay = delay_bw_A.front();

            ID_extension* id_extension = new ID_extension;
            trans->get_extension( id_extension ); 

            trans_bw_A.pop();
            phase_bw_A.pop();
            delay_bw_A.pop();

            unsigned int target_nr = decode_address_T( trans->get_address());
            unsigned int initiator_nr = decode_address_I( trans->get_address());
            cout << "T_bw  A Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
        
            wait(DELAY_ROUTER, SC_NS);
            status_bw_A.push(socket_target_A->nb_transport_bw(*trans, phase, delay));

            target_nr = decode_address_T( trans->get_address());
            initiator_nr = decode_address_I( trans->get_address());
            cout << "T_bw  A Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Initiator #" << hex << initiator_nr << " -> Router)" << endl;
            
            if (target_nr == 0xB){
                bw_t_end_B.notify(0,SC_NS);
            }

            else if(target_nr == 0xC){
                bw_t_end_C.notify(0,SC_NS);
            }

            else{
                cout << "T_bw A ERROR" << endl;
            }
        }
    }

    // Respuesta a Iniciador B desde Target
    void thread_process_to_bw_B(){
        while (true)
        {  
            wait(bw_t_B.default_event());

            tlm::tlm_generic_payload* trans = trans_bw_B.front();
            tlm::tlm_phase            phase = phase_bw_B.front();
            sc_time                   delay = delay_bw_B.front();

            ID_extension* id_extension = new ID_extension;
            trans->get_extension( id_extension ); 

            trans_bw_B.pop();
            phase_bw_B.pop();
            delay_bw_B.pop();

            unsigned int target_nr = decode_address_T( trans->get_address());
            unsigned int initiator_nr = decode_address_I( trans->get_address());
            cout << "T_bw  B Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;

            wait(DELAY_ROUTER, SC_NS);
            status_bw_B.push(socket_target_B->nb_transport_bw(*trans, phase, delay));

            target_nr = decode_address_T( trans->get_address());
            initiator_nr = decode_address_I( trans->get_address());
            cout << "T_bw  B Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Initiator #" << hex << initiator_nr << " -> Router)" << endl;
            
            if (target_nr == 0xA){
                bw_t_end_A.notify(0,SC_NS);
            }

            else if(target_nr == 0xC){
                bw_t_end_C.notify(0,SC_NS);
            }

            else{
                cout << "T_bw B ERROR" << endl;
            }
        }
    }

    // Respuesta a Iniciador C desde Target
    void thread_process_to_bw_C(){
        while (true)
        {
            wait(bw_t_C.default_event());

            tlm::tlm_generic_payload* trans = trans_bw_C.front();
            tlm::tlm_phase            phase = phase_bw_C.front();
            sc_time                   delay = delay_bw_C.front();

            ID_extension* id_extension = new ID_extension;
            trans->get_extension( id_extension ); 

            trans_bw_C.pop();
            phase_bw_C.pop();
            delay_bw_C.pop();

            unsigned int target_nr = decode_address_T( trans->get_address());
            unsigned int initiator_nr = decode_address_I( trans->get_address());
            cout << "T_bw  C Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            
            wait(DELAY_ROUTER, SC_NS);
            status_bw_C.push(socket_target_C->nb_transport_bw(*trans, phase, delay));

            target_nr = decode_address_T( trans->get_address());
            initiator_nr = decode_address_I( trans->get_address());
            cout << "T_bw  C Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Initiator #" << hex << initiator_nr << " -> Router)" << endl;

            if (target_nr == 0xA){
                bw_t_end_A.notify(0,SC_NS);
            }

            else if(target_nr == 0xB){
                bw_t_end_B.notify(0,SC_NS);
            }

            else{
                cout << "T_bw C ERROR" << endl;
            }
        }
    }


  //==============================================================================================
  //                                   FUNCIONES DE INICIADOR
  //==============================================================================================

    // Desde Initiator A
    virtual tlm::tlm_sync_enum nb_transport_fw_A( tlm::tlm_generic_payload& trans,
                                                    tlm::tlm_phase& phase,
                                                    sc_time& delay )
    {
        ID_extension* id_extension = new ID_extension;
        trans.get_extension( id_extension ); 

        unsigned int target_nr = decode_address_T( trans.get_address());
        unsigned int initiator_nr = decode_address_I( trans.get_address());
        cout << "tlm_f A Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Initiator #" << hex << initiator_nr << " -> Router)" << endl;

        tlm::tlm_generic_payload* trans_pending;

        if(target_nr == 0xB)
        {
            trans_fw_B.push(&trans);
            phase_fw_B.push(phase);
            delay_fw_B.push(delay);
            fw_t_B.notify(0,SC_NS);
            wait(fw_t_end_A.default_event());
            
            cout << "tlm_f A Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            Aux_fw_A = status_fw_B.front();
            status_fw_B.pop();
            return Aux_fw_A;
        }
        else if (target_nr == 0xC)
        {
            trans_fw_C.push(&trans);
            phase_fw_C.push(phase);
            delay_fw_C.push(delay);
            fw_t_C.notify(0,SC_NS);
            wait(fw_t_end_A.default_event());
            
            cout << "tlm_f A Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            Aux_fw_A = status_fw_C.front();
            status_fw_C.pop();
            return Aux_fw_A;
        }
        else
        {
            cout << "tlm_f A ERROR" << endl;
            return tlm::TLM_COMPLETED;
        }
    }

    // Desde Initiator B
    virtual tlm::tlm_sync_enum nb_transport_fw_B( tlm::tlm_generic_payload& trans,
                                                    tlm::tlm_phase& phase,
                                                    sc_time& delay )
    {
        ID_extension* id_extension = new ID_extension;
        trans.get_extension( id_extension ); 

        unsigned int target_nr = decode_address_T( trans.get_address());
        unsigned int initiator_nr = decode_address_I( trans.get_address());
        cout << "tlm_f B Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Initiator #" << hex << initiator_nr << " -> Router)" << endl;
        
        tlm::tlm_generic_payload* trans_pending;

        if(target_nr == 0xA)
        {
            trans_fw_A.push(&trans);
            phase_fw_A.push(phase);
            delay_fw_A.push(delay);
            fw_t_A.notify(0,SC_NS);
            wait(fw_t_end_B.default_event());

            cout << "tlm_f B Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            Aux_fw_B = status_fw_A.front();
            status_fw_A.pop();
            return Aux_fw_B;
        }
        else if (target_nr == 0xC)
        {
            trans_fw_C.push(&trans);
            phase_fw_C.push(phase);
            delay_fw_C.push(delay);
            fw_t_C.notify(0,SC_NS);
            wait(fw_t_end_B.default_event());
            
            cout << "tlm_f B Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            Aux_fw_B = status_fw_C.front();
            status_fw_C.pop();
            return Aux_fw_B;
        }
        else
        {
            cout << "tlm_f B ERROR" << endl;
            return tlm::TLM_COMPLETED;
        }
    }

    // Desde Initiator C
    virtual tlm::tlm_sync_enum nb_transport_fw_C( tlm::tlm_generic_payload& trans,
                                                    tlm::tlm_phase& phase,
                                                    sc_time& delay )
    {
        ID_extension* id_extension = new ID_extension;
        trans.get_extension( id_extension ); 

        unsigned int target_nr = decode_address_T( trans.get_address());
        unsigned int initiator_nr = decode_address_I( trans.get_address());
        cout << "tlm_f C Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Initiator #" << hex << initiator_nr << " -> Router)" << endl;
        
        tlm::tlm_generic_payload* trans_pending;

        if (target_nr == 0xA)
        {
            trans_fw_A.push(&trans);
            phase_fw_A.push(phase);
            delay_fw_A.push(delay);
            fw_t_A.notify(0,SC_NS);
            wait(fw_t_end_C.default_event());

            cout << "tlm_f C Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            Aux_fw_C = status_fw_A.front();
            status_fw_A.pop();
            return Aux_fw_C;
        }

        else if(target_nr == 0xB)
        {
            trans_fw_B.push(&trans);
            phase_fw_B.push(phase);
            delay_fw_B.push(delay);
            fw_t_B.notify(0,SC_NS);
            wait(fw_t_end_C.default_event());

            cout << "tlm_f C Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            Aux_fw_C = status_fw_B.front();
            status_fw_B.pop();
            return Aux_fw_C;
        }

        else
        {
            cout << "tlm_f C ERROR" << endl;
            return tlm::TLM_COMPLETED;
        }
    }

  //----------------------------------------------------------------------------------------------

    // Enviar paquete a Target A desde Iniciador
    void thread_process_to_fw_A(){
        while (true)
        {   
            wait(fw_t_A.default_event());

            tlm::tlm_generic_payload* trans = trans_fw_A.front();
            tlm::tlm_phase            phase = phase_fw_A.front();
            sc_time                   delay = delay_fw_A.front();

            trans_fw_A.pop();
            phase_fw_A.pop();
            delay_fw_A.pop();

            ID_extension* id_extension = new ID_extension;
            trans->get_extension( id_extension ); 

            unsigned int target_nr = decode_address_T( trans->get_address());
            unsigned int initiator_nr = decode_address_I( trans->get_address());
            cout << "t_fw  A Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            
            wait(DELAY_ROUTER, SC_NS);
            status_fw_A.push(socket_initiator_A->nb_transport_fw(*trans, phase, delay));
            
            target_nr = decode_address_T( trans->get_address());
            initiator_nr = decode_address_I( trans->get_address());
            cout << "t_fw  A Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Target #" << hex << target_nr << "    -> Router)" << endl;
            
            if (initiator_nr == 0xB){
                fw_t_end_B.notify(0,SC_NS);
            }

            else if(initiator_nr == 0xC){
                fw_t_end_C.notify(0,SC_NS);
            }

            else{
                cout << "t_fw A ERROR" << endl;
            }
        }
    }

    // Enviar paquete a Target B desde Iniciador
    void thread_process_to_fw_B(){
        while (true)
        {
            wait(fw_t_B.default_event());

            //Decode
            tlm::tlm_generic_payload* trans = trans_fw_B.front();
            tlm::tlm_phase            phase = phase_fw_B.front();
            sc_time                   delay = delay_fw_B.front();

            ID_extension* id_extension = new ID_extension;
            trans->get_extension( id_extension ); 

            trans_fw_B.pop();
            phase_fw_B.pop();
            delay_fw_B.pop();

            unsigned int target_nr = decode_address_T( trans->get_address());
            unsigned int initiator_nr = decode_address_I( trans->get_address());
            cout << "t_fw  B Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;

            wait(DELAY_ROUTER, SC_NS);
            status_fw_B.push(socket_initiator_B->nb_transport_fw(*trans, phase, delay));

            target_nr = decode_address_T( trans->get_address());
            initiator_nr = decode_address_I( trans->get_address());
            cout << "t_fw  B Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Target #" << hex << target_nr << "    -> Router)" << endl;
            
            if (initiator_nr == 0xA){
                fw_t_end_A.notify(0,SC_NS);
            }

            else if(initiator_nr == 0xC){
                fw_t_end_C.notify(0,SC_NS);
            }

            else{
                cout << "t_fw B ERROR" << endl;
            }
        }
    }

    // Enviar paquete a Target C desde Iniciador
    void thread_process_to_fw_C(){
        while (true)
        {
            wait(fw_t_C.default_event());

            //Decode
            tlm::tlm_generic_payload* trans = trans_fw_C.front();
            tlm::tlm_phase            phase = phase_fw_C.front();
            sc_time                   delay = delay_fw_C.front();

            ID_extension* id_extension = new ID_extension;
            trans->get_extension( id_extension ); 

            trans_fw_C.pop();
            phase_fw_C.pop();
            delay_fw_C.pop();

            unsigned int target_nr = decode_address_T( trans->get_address());
            unsigned int initiator_nr = decode_address_I( trans->get_address());
            cout << "t_fw  C Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;

            wait(DELAY_ROUTER, SC_NS);
            status_fw_C.push(socket_initiator_C->nb_transport_fw(*trans, phase, delay));

            target_nr = decode_address_T( trans->get_address());
            initiator_nr = decode_address_I( trans->get_address());
            cout << "t_fw  C Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Target #" << hex << target_nr << "    -> Router)" << endl;
            
            if (initiator_nr == 0xA){
                fw_t_end_A.notify(0,SC_NS);
            }

            else if(initiator_nr == 0xB){
                fw_t_end_B.notify(0,SC_NS);
            }

            else{
                cout << "t_fw C ERROR" << endl;
            }
        }
    }

    //----------------------------------------------------------------------------------------------------------------------
    // ****************
    // ROUTER INTERNALS
    // ****************

    // Simple fixed address decoding
    inline unsigned int decode_address_T( sc_dt::uint64 address )
    {
        unsigned int target_nr = static_cast<unsigned int>( (address >> 32) & 0xF );
        return target_nr;
    }

    // Simple fixed address decoding
    inline unsigned int decode_address_I( sc_dt::uint64 address )
    {
        unsigned int initiator_nr = static_cast<unsigned int>( (address >> 36) & 0xF );
        return initiator_nr;
    }

    //Iniciator A
    std::queue<tlm::tlm_generic_payload*> trans_fw_A;
    std::queue<tlm::tlm_phase>            phase_fw_A;
    std::queue<sc_time>                   delay_fw_A;

    std::queue<tlm::tlm_sync_enum>        status_fw_A;
    tlm::tlm_sync_enum                    Aux_fw_A;
    sc_event_queue                        fw_t_A;
    sc_event_queue                        fw_t_end_A;

    //Iniciator B
    std::queue<tlm::tlm_generic_payload*> trans_fw_B;
    std::queue<tlm::tlm_phase>            phase_fw_B;
    std::queue<sc_time>                   delay_fw_B;

    std::queue<tlm::tlm_sync_enum>        status_fw_B;
    tlm::tlm_sync_enum                    Aux_fw_B;
    sc_event_queue                        fw_t_B;
    sc_event_queue                        fw_t_end_B;

    //Iniciator C
    std::queue<tlm::tlm_generic_payload*> trans_fw_C;
    std::queue<tlm::tlm_phase>            phase_fw_C;
    std::queue<sc_time>                   delay_fw_C;

    std::queue<tlm::tlm_sync_enum>        status_fw_C;
    tlm::tlm_sync_enum                    Aux_fw_C;
    sc_event_queue                        fw_t_C;
    sc_event_queue                        fw_t_end_C;

    //Target A
    std::queue<tlm::tlm_generic_payload*> trans_bw_A;
    std::queue<tlm::tlm_phase>            phase_bw_A;
    std::queue<sc_time>                   delay_bw_A;

    std::queue<tlm::tlm_sync_enum>        status_bw_A;
    tlm::tlm_sync_enum                    Aux_bw_A;
    sc_event_queue                        bw_t_A;
    sc_event_queue                        bw_t_end_A;

    //Target B
    std::queue<tlm::tlm_generic_payload*> trans_bw_B;
    std::queue<tlm::tlm_phase>            phase_bw_B;
    std::queue<sc_time>                   delay_bw_B;

    std::queue<tlm::tlm_sync_enum>        status_bw_B;
    tlm::tlm_sync_enum                    Aux_bw_B;
    sc_event_queue                        bw_t_B;
    sc_event_queue                        bw_t_end_B;

    //Target C
    std::queue<tlm::tlm_generic_payload*> trans_bw_C;
    std::queue<tlm::tlm_phase>            phase_bw_C;
    std::queue<sc_time>                   delay_bw_C;

    std::queue<tlm::tlm_sync_enum>        status_bw_C;
    tlm::tlm_sync_enum                    Aux_bw_C;  
    sc_event_queue                        bw_t_C;
    sc_event_queue                        bw_t_end_C;
};

#endif