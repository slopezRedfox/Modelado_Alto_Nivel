#include "sc_device.hh"

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

using namespace sc_core;
using namespace std;

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

#define INT2U32(x) *(uint32_t*)&x
#define INT2U16(x) *(uint16_t*)&x

# define M_PI           3.14159265358979323846  /* pi */

#define DELAY_IP 5

//*************************************************************************
//*************************************************************************
//*************************************************************************

//Variables de puerto Target
sc_event_queue  do_target_t; 

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

sc_uint<16> adc_v;   // vector data from XADC
sc_uint<16> adc_i;   // vector data from XADC

bool  start;         // Active high, ready signal from estimador
sc_uint<32> param_1; // 32 bit vector output of the estimador
sc_uint<32> param_2; // 32 bit vector output of the estimador
sc_uint<32> volt;
sc_uint<32> current;

sc_event calc_t, done_IP;

float init_cond_1, init_cond_2; 
float p1, p2, p1_aux, p2_aux, y_log, I, V;

float Lambda = 3.99;                     	    //Short-Circuit current
float Psi = 5.1387085e-6;                	    //Is current (saturation)
float alpha = 0.625;                 			//Thermal voltage relation
float V_oc = 1/alpha*(log(Lambda/Psi));         //Open circuit voltage
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

//*************************************************************************
//*************************************************************************
//**************************************************************************/

Device::Device(sc_core::sc_module_name name,
    bool debug,
    unsigned long long int size,
    unsigned int offset) :
    socket("socket"),
    transaction_in_progress(0),
    response_in_progress(false),
    next_response_pending(0),
    end_req_pending(0),
    m_peq(this, &Device::peq_cb),
    debug(debug),
    size(size),
    offset(offset){
    

    /* Register tlm transport functions */
    socket.register_b_transport(this, &Device::b_transport);
    socket.register_transport_dbg(this, &Device::transport_dbg);
    socket.register_nb_transport_fw(this, &Device::nb_transport_fw);


    /* allocate storage memory */
    mem = new unsigned char[size];

    //SC_THREAD(TB);

    SC_METHOD(execute_transaction_process);
    sensitive << target_done_event;
    dont_initialize();
}

void Device::check_address(unsigned long long int addr){

    if (addr < offset || addr >= offset + size)
        SC_REPORT_FATAL("Device", "Address out of range. Did you set an "
                                  "appropriate size and offset?");
}

void Device::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay){
    
    /* Execute the read or write commands */
    execute_transaction(trans);
}

unsigned int Device::transport_dbg(tlm::tlm_generic_payload& trans){

    check_address(trans.get_address());

    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address() - offset;
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();

    unsigned char *mem_array_ptr = mem + adr;

    /* Load / Store the access: */
    cout << "Comando de: "<< cmd << endl;
    if ( cmd == tlm::TLM_READ_COMMAND ) {
        if (debug) {
            SC_REPORT_INFO("target", "DBG tlm::TLM_READ_COMMAND");
        }
        std::memcpy(ptr, mem_array_ptr, len);
    } else if ( cmd == tlm::TLM_WRITE_COMMAND ) {
        if (debug) {
            SC_REPORT_INFO("target", "DBG tlm::TLM_WRITE_COMMAND");
        }
        std::memcpy(mem_array_ptr, ptr, len);
    }

    return len;
}


/* TLM-2 non-blocking transport method */
tlm::tlm_sync_enum Device::nb_transport_fw(tlm::tlm_generic_payload& trans,
                                           tlm::tlm_phase& phase,
                                           sc_time& delay){
    /* Queue the transaction until the annotated time has elapsed */
    m_peq.notify(trans, phase, delay);
    return tlm::TLM_ACCEPTED;
}

void Device::peq_cb(tlm::tlm_generic_payload& trans,
               const tlm::tlm_phase& phase){
    
    sc_time delay;

    if (phase == tlm::BEGIN_REQ) {
        if (debug) SC_REPORT_INFO("target", "tlm::BEGIN_REQ");

        /* Increment the transaction reference count */
        trans.acquire();

        if ( !transaction_in_progress ) {
            send_end_req(trans);
        } else {
            /* Put back-pressure on initiator by deferring END_REQ until
             * pipeline is clear */
            end_req_pending = &trans;
        }
    } else if (phase == tlm::END_RESP) {
        /* On receiving END_RESP, the target can release the transaction and
         * allow other pending transactions to proceed */
        if (!response_in_progress) {
            SC_REPORT_FATAL("TLM-2", "Illegal transaction phase END_RESP"
                            "received by target");
        }

        transaction_in_progress = 0;

        /* Device itself is now clear to issue the next BEGIN_RESP */
        response_in_progress = false;
        if (next_response_pending) {
            send_response( *next_response_pending );
            next_response_pending = 0;
        }

        /* ... and to unblock the initiator by issuing END_REQ */
        if (end_req_pending) {
            send_end_req( *end_req_pending );
            end_req_pending = 0;
        }

    } else /* tlm::END_REQ or tlm::BEGIN_RESP */ {
            SC_REPORT_FATAL("TLM-2", "Illegal transaction phase received by"
                            "target");
    }
}

void Device::send_end_req(tlm::tlm_generic_payload& trans){
    
    tlm::tlm_phase bw_phase;
    sc_time delay;

    /* Queue the acceptance and the response with the appropriate latency */
    bw_phase = tlm::END_REQ;
    delay = sc_time(10.0, SC_NS); // Accept delay

    tlm::tlm_sync_enum status;
    status = socket->nb_transport_bw(trans, bw_phase, delay);

    /* Ignore return value;
     * initiator cannot terminate transaction at this point
     * Queue internal event to mark beginning of response: */
    delay = delay + sc_time(40.0, SC_NS); // Latency
    target_done_event.notify(delay);

    assert(transaction_in_progress == 0);
    transaction_in_progress = &trans;
}

void Device::execute_transaction_process(){

    /* Execute the read or write commands */
    execute_transaction( *transaction_in_progress );

    /* Device must honor BEGIN_RESP/END_RESP exclusion rule; i.e. must not
     * send BEGIN_RESP until receiving previous END_RESP or BEGIN_REQ */
    if (response_in_progress) {
        /* Device allows only two transactions in-flight */
        if (next_response_pending) {
            SC_REPORT_FATAL("TLM-2", "Attempt to have two pending responses"
                            "in target");
        }
        next_response_pending = transaction_in_progress;
    } else {
        send_response( *transaction_in_progress );
    }
}

void Device::execute_transaction(tlm::tlm_generic_payload& trans){

    check_address(trans.get_address());

    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64    adr = trans.get_address() - offset;
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    if ( byt != 0 ) {
        cout << "Byte Error" << endl;
        trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
        return;
    }

    unsigned char *mem_array_ptr = mem + adr;
    unsigned char *Aux_addr      = mem + 0x1ff00000;
    int Aux;

    /* Load / Store the access: */
    //cout << "Comando Execute_transaction: " << cmd << endl;
    if ( cmd == tlm::TLM_READ_COMMAND ) {
        cout << "READ COMAND" << endl;

        if (debug) {
            SC_REPORT_INFO("target", "tlm::TLM_READ_COMMAND");
        }
        //DO SOMETHING DIFFERENT HERE FOR YOUR PROJECT
        std::memcpy(ptr, mem_array_ptr, len);
    }

    else if ( cmd == tlm::TLM_WRITE_COMMAND ) {

        cout << "WRITE COMAND" << endl;
        cout << "addr: " << adr << endl;

        std::memcpy(mem_array_ptr, ptr, len);
        std::memcpy(&Aux, Aux_addr, 4);

        cout << "data: " << Aux << endl;

        /*unsigned char * see_me = mem + 0x1ff00008;
        std::memcpy(&Aux, see_me, len);
        if (Aux == 1){
            start = 1;
        }*/
    }

    trans.set_response_status( tlm::TLM_OK_RESPONSE );
}

void Device::send_response(tlm::tlm_generic_payload& trans) {

    tlm::tlm_sync_enum status;
    tlm::tlm_phase bw_phase;
    sc_time delay;

    response_in_progress = true;
    bw_phase = tlm::BEGIN_RESP;
    delay = sc_time(10.0, SC_NS);
    status = socket->nb_transport_bw( trans, bw_phase, delay );

    if (status == tlm::TLM_UPDATED) {
        /* The timing annotation must be honored */
        m_peq.notify(trans, bw_phase, delay);
    } else if (status == tlm::TLM_COMPLETED) {
        /* The initiator has terminated the transaction */
        transaction_in_progress = 0;
        response_in_progress = false;
    }
    trans.release();
}

/*void  Device::TB(){
    if (start){
        cout << "Start" << endl;
    }
}*/