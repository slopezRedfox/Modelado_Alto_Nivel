#include "sc_router.hh"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

template<unsigned int N_TARGETS>
Router<N_TARGETS>::Router(sc_core::sc_module_name name,
    bool debug):
    socket("socket")
{
    /* Register tlm transport functions */
    socket.register_b_transport(this, &Router::b_transport);
    socket.register_transport_dbg(this, &Router::transport_dbg);
    socket.register_nb_transport_fw(this, &Router::nb_transport_fw);
    
    for (int i = 0; i < N_TARGETS; i++)
    {
      char txt[20];
      sprintf(txt, "socket_%d", i);
      initiator_socket[i] = new tlm_utils::simple_initiator_socket_tagged<Router>(txt);
      // Register callbacks for incoming interface method calls
      initiator_socket[i]->register_nb_transport_bw(this, &Router::nb_transport_bw, i);
    }

}

// TLM-2 backward non-blocking transport method
template<unsigned int N_TARGETS>
tlm::tlm_sync_enum Router<N_TARGETS>::nb_transport_bw(int id, tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_time& delay )
  {
    socket->nb_transport_bw(trans, phase, delay);
  }


// TLM-2 blocking transport method
template<unsigned int N_TARGETS>
void Router<N_TARGETS>::b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
  {
    sc_dt::uint64 address = trans.get_address();
    unsigned int target_nr;
    
    if (address < 0x1ff00000) target_nr=0;
    else {
        target_nr=1;
        printf("Accessing Device Memory Region\n");
        }
    // Forward transaction to appropriate target
    ( *initiator_socket[target_nr] )->b_transport( trans, delay );
  }


template<unsigned int N_TARGETS>
unsigned int Router<N_TARGETS>::transport_dbg(tlm::tlm_generic_payload& trans)
{
    sc_dt::uint64 address = trans.get_address();
    unsigned int target_nr;

    if (address < 0x1ff00000) target_nr=0;
    else {
        target_nr=1;
        printf("Accessing Device Memory Region\n");
    }    
    // Forward transaction to appropriate target
    ( *initiator_socket[target_nr] )->transport_dbg( trans );  

}
/* TLM-2 non-blocking transport method */
template<unsigned int N_TARGETS>
tlm::tlm_sync_enum Router<N_TARGETS>::nb_transport_fw(tlm::tlm_generic_payload& trans,
                                           tlm::tlm_phase& phase,
                                           sc_time& delay)
{
    sc_dt::uint64 address = trans.get_address();
    unsigned int target_nr;

    if (address < 0x1ff00000) target_nr=0;
    else {
        target_nr=1;
        printf("Accessing Device Memory Region\n");
    }
    // Forward transaction to appropriate target
    ( *initiator_socket[target_nr] )->nb_transport_fw( trans, phase, delay );  
}

