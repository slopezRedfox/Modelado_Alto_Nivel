// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include <queue>

// *********************************************
// Generic payload blocking transport router
// *********************************************

struct Router: sc_module{

  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm_utils::simple_initiator_socket<Router> socket_to_target_1;
  tlm_utils::simple_initiator_socket<Router> socket_to_target_2;
  tlm_utils::simple_target_socket<Router>    socket_to_initiator;

  SC_CTOR(Router): 
    socket_to_target_1("socket_to_target_1"),
    socket_to_target_2("socket_to_target_2"),
    socket_to_initiator("socket_to_initiator")
  {
    // Register callbacks for incoming interface method calls
    socket_to_target_1.register_nb_transport_bw(this, &Router::nb_transport_bw_2);
    socket_to_target_2.register_nb_transport_bw(this, &Router::nb_transport_bw_1);
    socket_to_initiator.register_nb_transport_fw(this, &Router::nb_transport_fw);   
    //SC_THREAD(thread_process_to_fw_2);
    SC_THREAD(thread_process_to_fw);
    SC_THREAD(thread_process_to_bw);
  }

  //----------------------------------------------------------------------------------------------------------------------
  
  //Respuesta a Iniciador
  void thread_process_to_bw(){
    while (true){
      wait(bw_t.default_event());
      printf("Send    from Router (Router    -> Initiator)\n");
      status_bw = socket_to_initiator->nb_transport_bw(*trans_bw.front(), phase_bw.front(), delay_bw.front());
      trans_bw.pop();
      phase_bw.pop();
      delay_bw.pop();

      printf("Receive into Router (Initiator -> Router)\n");
      bw_t_end.notify();
    }
  }

  //Respuesta a Targets
  void thread_process_to_fw(){
    while (true)
    {
      wait(fw_t.default_event());
      printf("Send    from Router (Router    -> Target)\n");

      //Decode
      tlm::tlm_generic_payload* trans;
      trans = trans_fw.front();

      unsigned int target_nr = decode_address( trans->get_address());
      cout << "******************************" << endl;
      cout << "****         Port #" << target_nr << "      ****" << endl;
      cout << "******************************" << endl;

      if(target_nr == 0){
        status_fw = socket_to_target_1->nb_transport_fw(*trans_fw.front(), phase_fw.front(), delay_fw.front());
      }
      else if (target_nr == 1){
        status_fw = socket_to_target_2->nb_transport_fw(*trans_fw.front(), phase_fw.front(), delay_fw.front());
      }
      else{
        cout << "ERROR" << endl;
      }

      trans_fw.pop();
      phase_fw.pop();
      delay_fw.pop();

      printf("Receive into Router (Target    -> Router)\n");
      fw_t_end.notify();
    }
  }

  //----------------------------------------------------------------------------------------------------------------------

  //Target #1
  virtual tlm::tlm_sync_enum nb_transport_bw_1( tlm::tlm_generic_payload& trans,
                                                tlm::tlm_phase& phase,
                                                sc_time& delay ){
    printf("Receive into Router (Target    -> Router)\n");
    tlm::tlm_generic_payload* trans_pending;   
    trans_bw.push(&trans);
    phase_bw.push(phase);
    delay_bw.push(delay);

    bw_t.notify(1,SC_NS);
    wait(bw_t_end);
    printf("Send    from Router (Router    -> Target)\n");
    return status_bw;
  }

  //Target #2
  virtual tlm::tlm_sync_enum nb_transport_bw_2( tlm::tlm_generic_payload& trans,
                                                tlm::tlm_phase& phase,
                                                sc_time& delay ){
    printf("Receive into Router (Target    -> Router)\n");
    tlm::tlm_generic_payload* trans_pending;   
    trans_bw.push(&trans);
    phase_bw.push(phase);
    delay_bw.push(delay);

    bw_t.notify(1,SC_NS);
    wait(bw_t_end);
    printf("Send    from Router (Router    -> Target)\n");
    return status_bw;
  }

  //Initiator
  virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase,
                                              sc_time& delay ){

    printf("Receive into Router (Initiator -> Router)\n");
    tlm::tlm_generic_payload* trans_pending;   
    trans_fw.push(&trans);
    phase_fw.push(phase);
    delay_fw.push(delay);

    fw_t.notify(1,SC_NS);
    wait(fw_t_end);
    printf("Send    from Router (Router    -> Initiator)\n");
    return status_fw;
  }


  //----------------------------------------------------------------------------------------------------------------------
  // ****************
  // ROUTER INTERNALS
  // ****************

  // Simple fixed address decoding
  inline unsigned int decode_address( sc_dt::uint64 address )
  {
    unsigned int target_nr = static_cast<unsigned int>( (address >> 32) & 0x3 );
    return target_nr;
  }

  inline sc_dt::uint64 compose_address( unsigned int target_nr, sc_dt::uint64 address)
  {
    return (target_nr << 8) | (address & 0xFF);
  }

  std::queue<tlm::tlm_generic_payload*> trans_fw;
  std::queue<tlm::tlm_phase>            phase_fw;
  std::queue<sc_time>                   delay_fw;

  std::queue<tlm::tlm_generic_payload*> trans_bw;
  std::queue<tlm::tlm_phase>            phase_bw;
  std::queue<sc_time>                   delay_bw;

  tlm::tlm_sync_enum status_bw, status_fw;
  sc_event_queue  bw_t, fw_t;
  sc_event  bw_t_end, fw_t_end;
};
