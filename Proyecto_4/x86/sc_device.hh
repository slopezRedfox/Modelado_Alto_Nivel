#ifndef __SIM_SC_DEVICE_HH__
#define __SIM_SC_DEVICE_HH__

#include <tlm_utils/peq_with_cb_and_phase.h>
#include <tlm_utils/simple_target_socket.h>

#include <iostream>
#include <systemc>
#include <tlm>

using namespace sc_core;
using namespace std;

struct Device: sc_module
{
    /** TLM interface socket: */
    tlm_utils::simple_target_socket<Device> socket;

    /** TLM related member variables: */
    tlm::tlm_generic_payload*  transaction_in_progress;
    sc_event                   target_done_event;
    bool                       response_in_progress;
    bool                       debug;
    tlm::tlm_generic_payload*  next_response_pending;
    tlm::tlm_generic_payload*  end_req_pending;
    tlm_utils::peq_with_cb_and_phase<Device> m_peq;

    /** Storage, may be implemented with a map for large devices */
    unsigned char *mem;

    Device(sc_core::sc_module_name name,
        bool debug,
        unsigned long long int size,
        unsigned int offset);
    SC_HAS_PROCESS(Device);

    /** TLM interface functions */
    virtual void b_transport(tlm::tlm_generic_payload& trans,
                             sc_time& delay);
    virtual unsigned int transport_dbg(tlm::tlm_generic_payload& trans);
    virtual tlm::tlm_sync_enum nb_transport_fw(
                tlm::tlm_generic_payload& trans,
                tlm::tlm_phase& phase,
                sc_time& delay);

    /** Callback of Payload Event Queue: */
    void peq_cb(tlm::tlm_generic_payload& trans,
                const tlm::tlm_phase& phase);

    /** Helping function common to b_transport and nb_transport */
    void execute_transaction(tlm::tlm_generic_payload& trans);

    /** Helping functions and processes: */
    void send_end_req(tlm::tlm_generic_payload& trans);
    void send_response(tlm::tlm_generic_payload& trans);

    /** Method process that runs on target_done_event */
    void execute_transaction_process();

    void tb();
    void estimador_main();

    /** Helping function that checks if a requested address is with range */
    void check_address(unsigned long long int addr);

    /** Helping Variables **/
    unsigned long long int size;
    unsigned offset;
};

#endif //__SIM_SC_DEVICE_HH__

