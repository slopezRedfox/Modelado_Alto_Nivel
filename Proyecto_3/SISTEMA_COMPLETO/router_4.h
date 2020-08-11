#ifndef ROUTER4_H
#define ROUTER4_H

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

struct Router4: sc_module{

    tlm_utils::simple_initiator_socket<Router4> socket_initiator_A;
    tlm_utils::simple_target_socket<Router4>    socket_target_A;

    tlm_utils::simple_initiator_socket<Router4> socket_initiator_E;
    tlm_utils::simple_target_socket<Router4>    socket_target_E;

    tlm_utils::simple_initiator_socket<Router4> socket_initiator_C;
    tlm_utils::simple_target_socket<Router4>    socket_target_C;

    SC_CTOR(Router4):
        socket_initiator_A("socket_initiator_A"),
        socket_target_A("socket_target_A"),

        socket_initiator_E("socket_initiator_E"),
        socket_target_E("socket_target_E"),

        socket_initiator_C("socket_initiator_C"),
        socket_target_C("socket_target_C")
    {

        socket_initiator_A.register_nb_transport_bw(this, &Router4::nb_transport_bw_A);
        socket_target_A.register_nb_transport_fw(this, &Router4::nb_transport_fw_A);

        socket_initiator_E.register_nb_transport_bw(this, &Router4::nb_transport_bw_E);
        socket_target_E.register_nb_transport_fw(this, &Router4::nb_transport_fw_E);

        socket_initiator_C.register_nb_transport_bw(this, &Router4::nb_transport_bw_C);
        socket_target_C.register_nb_transport_fw(this, &Router4::nb_transport_fw_C);

        SC_THREAD(thread_process_to_bw_A);
        SC_THREAD(thread_process_to_fw_A);

        SC_THREAD(thread_process_to_bw_E);
        SC_THREAD(thread_process_to_fw_E);

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

        if(initiator_nr == 0xE)
        {
            trans_bw_E.push(&trans);
            phase_bw_E.push(phase);
            delay_bw_E.push(delay);
            bw_t_E.notify(0,SC_NS);
            wait(bw_t_end_A.default_event());
            
            cout << "tlm_b A Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            Aux_bw_A = status_bw_E.front();
            status_bw_E.pop();
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

    // Desde Target #E
    virtual tlm::tlm_sync_enum nb_transport_bw_E( tlm::tlm_generic_payload& trans,
                                                    tlm::tlm_phase& phase,
                                                    sc_time& delay )
    {
        ID_extension* id_extension = new ID_extension;
        trans.get_extension( id_extension ); 

        unsigned int target_nr = decode_address_T( trans.get_address());
        unsigned int initiator_nr = decode_address_I( trans.get_address());
        cout << "tlm_b E Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Target #" << hex << target_nr << "    -> Router)" << endl;

        tlm::tlm_generic_payload* trans_pending;   

        if(initiator_nr == 0xA)
        {
            trans_bw_A.push(&trans);
            phase_bw_A.push(phase);
            delay_bw_A.push(delay);
            bw_t_A.notify(0,SC_NS);
            wait(bw_t_end_E.default_event());
            
            cout << "tlm_b E Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            Aux_bw_E = status_bw_A.front();
            status_bw_A.pop();
            return Aux_bw_E;
        }
        
        else if(initiator_nr == 0xC)
        {
            trans_bw_C.push(&trans);
            phase_bw_C.push(phase);
            delay_bw_C.push(delay);
            bw_t_C.notify(0,SC_NS);
            wait(bw_t_end_E.default_event());
            
            cout << "tlm_b E Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            Aux_bw_E = status_bw_C.front();
            status_bw_C.pop();
            return Aux_bw_E;
        }
        else
        {
            cout << "tlm_b E ERROR" << endl;
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
        
        else if(initiator_nr == 0xE)
        {
            trans_bw_E.push(&trans);
            phase_bw_E.push(phase);
            delay_bw_E.push(delay);
            bw_t_E.notify(0,SC_NS);
            wait(bw_t_end_C.default_event());
            
            cout << "tlm_b C Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;
            Aux_bw_C = status_bw_E.front();
            status_bw_E.pop();
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
            
            if (target_nr == 0xE){
                bw_t_end_E.notify(0,SC_NS);
            }

            else if(target_nr == 0xC){
                bw_t_end_C.notify(0,SC_NS);
            }

            else{
                cout << "T_bw A ERROR" << endl;
            }
        }
    }

    // Respuesta a Iniciador E desde Target
    void thread_process_to_bw_E(){
        while (true)
        {  
            wait(bw_t_E.default_event());

            tlm::tlm_generic_payload* trans = trans_bw_E.front();
            tlm::tlm_phase            phase = phase_bw_E.front();
            sc_time                   delay = delay_bw_E.front();

            ID_extension* id_extension = new ID_extension;
            trans->get_extension( id_extension ); 

            trans_bw_E.pop();
            phase_bw_E.pop();
            delay_bw_E.pop();

            unsigned int target_nr = decode_address_T( trans->get_address());
            unsigned int initiator_nr = decode_address_I( trans->get_address());
            cout << "T_bw  E Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;

            wait(DELAY_ROUTER, SC_NS);
            status_bw_E.push(socket_target_E->nb_transport_bw(*trans, phase, delay));

            target_nr = decode_address_T( trans->get_address());
            initiator_nr = decode_address_I( trans->get_address());
            cout << "T_bw  E Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Initiator #" << hex << initiator_nr << " -> Router)" << endl;
            
            if (target_nr == 0xA){
                bw_t_end_A.notify(0,SC_NS);
            }

            else if(target_nr == 0xC){
                bw_t_end_C.notify(0,SC_NS);
            }

            else{
                cout << "T_bw E ERROR" << endl;
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

            else if(target_nr == 0xE){
                bw_t_end_E.notify(0,SC_NS);
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

        if(target_nr == 0xE)
        {
            trans_fw_E.push(&trans);
            phase_fw_E.push(phase);
            delay_fw_E.push(delay);
            fw_t_E.notify(0,SC_NS);
            wait(fw_t_end_A.default_event());
            
            cout << "tlm_f A Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            Aux_fw_A = status_fw_E.front();
            status_fw_E.pop();
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

    // Desde Initiator E
    virtual tlm::tlm_sync_enum nb_transport_fw_E( tlm::tlm_generic_payload& trans,
                                                    tlm::tlm_phase& phase,
                                                    sc_time& delay )
    {
        ID_extension* id_extension = new ID_extension;
        trans.get_extension( id_extension ); 

        unsigned int target_nr = decode_address_T( trans.get_address());
        unsigned int initiator_nr = decode_address_I( trans.get_address());
        cout << "tlm_f E Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Initiator #" << hex << initiator_nr << " -> Router)" << endl;
        
        tlm::tlm_generic_payload* trans_pending;

        if(target_nr == 0xA)
        {
            trans_fw_A.push(&trans);
            phase_fw_A.push(phase);
            delay_fw_A.push(delay);
            fw_t_A.notify(0,SC_NS);
            wait(fw_t_end_E.default_event());

            cout << "tlm_f E Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            Aux_fw_E = status_fw_A.front();
            status_fw_A.pop();
            return Aux_fw_E;
        }
        else if (target_nr == 0xC)
        {
            trans_fw_C.push(&trans);
            phase_fw_C.push(phase);
            delay_fw_C.push(delay);
            fw_t_C.notify(0,SC_NS);
            wait(fw_t_end_E.default_event());
            
            cout << "tlm_f E Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            Aux_fw_E = status_fw_C.front();
            status_fw_C.pop();
            return Aux_fw_E;
        }
        else
        {
            cout << "tlm_f E ERROR" << endl;
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

        else if(target_nr == 0xE)
        {
            trans_fw_E.push(&trans);
            phase_fw_E.push(phase);
            delay_fw_E.push(delay);
            fw_t_E.notify(0,SC_NS);
            wait(fw_t_end_C.default_event());

            cout << "tlm_f C Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Initiator #" << hex << initiator_nr << ")" << endl;
            Aux_fw_C = status_fw_E.front();
            status_fw_E.pop();
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
            
            if (initiator_nr == 0xE){
                fw_t_end_E.notify(0,SC_NS);
            }

            else if(initiator_nr == 0xC){
                fw_t_end_C.notify(0,SC_NS);
            }

            else{
                cout << "t_fw A ERROR" << endl;
            }
        }
    }

    // Enviar paquete a Target E desde Iniciador
    void thread_process_to_fw_E(){
        while (true)
        {
            wait(fw_t_E.default_event());

            //Decode
            tlm::tlm_generic_payload* trans = trans_fw_E.front();
            tlm::tlm_phase            phase = phase_fw_E.front();
            sc_time                   delay = delay_fw_E.front();

            ID_extension* id_extension = new ID_extension;
            trans->get_extension( id_extension ); 

            trans_fw_E.pop();
            phase_fw_E.pop();
            delay_fw_E.pop();

            unsigned int target_nr = decode_address_T( trans->get_address());
            unsigned int initiator_nr = decode_address_I( trans->get_address());
            cout << "t_fw  E Send    from Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Router       -> Target #" << hex << target_nr << ")" << endl;

            wait(DELAY_ROUTER, SC_NS);
            status_fw_E.push(socket_initiator_E->nb_transport_fw(*trans, phase, delay));

            target_nr = decode_address_T( trans->get_address());
            initiator_nr = decode_address_I( trans->get_address());
            cout << "t_fw  E Receive into Router " << name() << " TRANS ID " << hex << id_extension->transaction_id << " Phase: " << phase << " at time " << sc_time_stamp() << " (Target #" << hex << target_nr << "    -> Router)" << endl;
            
            if (initiator_nr == 0xA){
                fw_t_end_A.notify(0,SC_NS);
            }

            else if(initiator_nr == 0xC){
                fw_t_end_C.notify(0,SC_NS);
            }

            else{
                cout << "t_fw E ERROR" << endl;
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

            else if(initiator_nr == 0xE){
                fw_t_end_E.notify(0,SC_NS);
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

    //Iniciator E
    std::queue<tlm::tlm_generic_payload*> trans_fw_E;
    std::queue<tlm::tlm_phase>            phase_fw_E;
    std::queue<sc_time>                   delay_fw_E;

    std::queue<tlm::tlm_sync_enum>        status_fw_E;
    tlm::tlm_sync_enum                    Aux_fw_E;
    sc_event_queue                        fw_t_E;
    sc_event_queue                        fw_t_end_E;

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

    //Target E
    std::queue<tlm::tlm_generic_payload*> trans_bw_E;
    std::queue<tlm::tlm_phase>            phase_bw_E;
    std::queue<sc_time>                   delay_bw_E;

    std::queue<tlm::tlm_sync_enum>        status_bw_E;
    tlm::tlm_sync_enum                    Aux_bw_E;
    sc_event_queue                        bw_t_E;
    sc_event_queue                        bw_t_end_E;

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