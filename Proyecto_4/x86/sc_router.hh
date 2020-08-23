#ifndef __SIM_SC_ROUTER_HH__
#define __SIM_SC_ROUTER_HH__

#include <tlm_utils/peq_with_cb_and_phase.h>
#include <tlm_utils/simple_target_socket.h>
#include "tlm_utils/simple_initiator_socket.h"

#include <iostream>
#include <systemc>
#include <tlm>

using namespace sc_core;
using namespace sc_dt;
using namespace std;

template<unsigned int N_TARGETS>
struct Router: sc_module
{
    /** TLM interface socket: */
    tlm_utils::simple_target_socket<Router> socket;
    
    // *********************************************
    // Use tagged sockets to be able to distinguish incoming backward path calls
    // *********************************************

    tlm_utils::simple_initiator_socket_tagged<Router>* initiator_socket[N_TARGETS];

    /** TLM related member variables: */
    bool                       debug;
        
    SC_HAS_PROCESS(Router);
    Router(sc_core::sc_module_name name,
        bool debug);
   

    /** TLM interface functions */
    virtual void b_transport(tlm::tlm_generic_payload& trans,
                             sc_time& delay);
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans);
    virtual tlm::tlm_sync_enum nb_transport_fw(
                tlm::tlm_generic_payload& trans,
                tlm::tlm_phase& phase,
                sc_time& delay);

    virtual tlm::tlm_sync_enum nb_transport_bw(
                int id,
                tlm::tlm_generic_payload& trans,
                tlm::tlm_phase& phase, 
                sc_time& delay );
};

#endif //__SIM_SC_ROUTER_HH__

