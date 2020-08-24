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
#define I_scale_factor_Addr 0x1ff00020
#define V_scale_factor_Addr 0x1ff00024
#define Ig_value_Addr       0x1ff00028
#define Gamma11_Addr        0x1ff0002c
#define Gamma12_Addr        0x1ff00030
#define Gamma21_Addr        0x1ff00038
#define Gamma22_Addr        0x1ff00040
#define Init_alpha_Addr     0x1ff00048
#define Init_beta_Addr      0x1ff00050
#define T_sampling_Addr     0x1ff00058
#define Start_Addr          0x1ff00060
#define p1_Addr             0x1ff00064
#define p2_Addr             0x1ff00068


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

#define DELAY_IP 5

//*************************************************************************
//*************************************************************************
//*************************************************************************/

//Variables internas
float I_scale_factor_e, V_scale_factor_e, Ig_e, GAMMA11_e, GAMMA12_e, GAMMA21_e, GAMMA22_e, INIT_ALPHA_e, INIT_BETA_e, T_SAMPLING_e;

sc_uint<16> adc_v;   // vector data from XADC
sc_uint<16> adc_i;   // vector data from XADC

int   start;         // Active high, ready signal from estimador
sc_uint<32> param_1; // 32 bit vector output of the estimador
sc_uint<32> param_2; // 32 bit vector output of the estimador
sc_uint<32> volt;
sc_uint<32> current;

float init_cond_1, init_cond_2;
float p1, p2, p1_aux, p2_aux, y_log, I, V;

float Lambda = 3.99;                     	        //Short-Circuit current
float Psi = 5.1387085e-6;                	        //Is current (saturation)
float alpha = 0.625;                 			//Thermal voltage relation
float V_oc = 1/alpha*(log(Lambda/Psi));                 //Open circuit voltage
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

    SC_METHOD(estimador_main);
    sensitive << calc_t;
    dont_initialize();

    //SC_METHOD(tb);
    SC_THREAD(tb);
    //sensitive << tb_do_event;
    //dont_initialize();

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
    unsigned char *Aux_addr_1    = mem + 0x1ff00000;
    int Aux_1;
    int Aux_2;

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
        //cout << "len : " << len << endl;

        std::memcpy(mem_array_ptr, ptr, len);
        std::memcpy(&Aux_1, Aux_addr_1, 4);

        std::memcpy(&Aux_2,mem + I_scale_factor_Addr, 4);
        I_scale_factor_e = Aux_2/pow(2,21);

        std::memcpy(&Aux_2,mem + V_scale_factor_Addr, 4);
        V_scale_factor_e = Aux_2/pow(2,21);

        std::memcpy(&Aux_2,mem + Ig_value_Addr, 4);
        Ig_e             = Aux_2/pow(2,21);

        std::memcpy(&Aux_2,mem + Gamma11_Addr, 4);
        GAMMA11_e        = Aux_2/pow(2,21);

        std::memcpy(&Aux_2,mem + Gamma12_Addr, 4);
        GAMMA12_e        = Aux_2/pow(2,21);

        std::memcpy(&Aux_2,mem + Gamma21_Addr, 4);
        GAMMA21_e        = Aux_2/pow(2,21);

        std::memcpy(&Aux_2,mem + Gamma22_Addr, 4);
        GAMMA22_e        = Aux_2/pow(2,21);

        std::memcpy(&Aux_2,mem + Init_alpha_Addr, 4);
        INIT_ALPHA_e     = Aux_2/pow(2,21);

        std::memcpy(&Aux_2,mem + Init_beta_Addr, 4);
        INIT_BETA_e      = Aux_2/pow(2,21);

        std::memcpy(&Aux_2,mem + T_sampling_Addr, 4);
        T_SAMPLING_e     = Aux_2/pow(2,21);

        std::memcpy(&Aux_2,mem + Start_Addr, 4);
        start            = Aux_2;

        cout << "data1: " << Aux_1 << endl;
        if (start == 0x1){
            start = 1;
            tb_do_event.notify();
        }
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

//================================================================
//=====================     MAIN PART OF IP  =====================
//================================================================
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

//Funcion estimador_main
void  Device::estimador_main(){

    if (start){
        cout << endl << "       ***************" << endl;
        cout << "         Enter Start" << endl;
        cout << "       ***************" << endl;
        cout << "*******************************"      << endl;
        cout << "       === SUMMERY ==="               << endl;
        cout << "I_scale_factor: " << dec << I_scale_factor_e << endl;
        cout << "V_scale_factor: " << dec << V_scale_factor_e << endl;
        cout << "Ig            : " << dec << Ig_e             << endl;
        cout << "GAMMA11       : " << dec << GAMMA11_e        << endl;
        cout << "GAMMA12       : " << dec << GAMMA12_e        << endl;
        cout << "GAMMA21       : " << dec << GAMMA21_e        << endl;
        cout << "GAMMA22       : " << dec << GAMMA22_e        << endl;
        cout << "INIT_ALPHA    : " << dec << INIT_ALPHA_e     << endl;
        cout << "INIT_BETA     : " << dec << INIT_BETA_e      << endl;
        cout << "T_SAMPLING    : " << dec << T_SAMPLING_e     << endl;
        cout << "*******************************"      << endl << endl;

        init_cond_1 = INIT_ALPHA_e;
        init_cond_2 = INIT_BETA_e;
        start = 0;
    }

    I = adc_i / pow(2,16);
    V = adc_v / pow(2,16);

    I *= I_scale_factor;
    V *= V_scale_factor;
    y_log = log(Ig - I);
    p1=((GAMMA11*V+GAMMA12)*(y_log-(V*init_cond_1)-init_cond_2))*T_SAMPLING+init_cond_1;
    p2=((GAMMA21*V+GAMMA22)*(y_log-(V*init_cond_1)-init_cond_2))*T_SAMPLING+init_cond_2;
    init_cond_1=p1;
    init_cond_2=p2;

    param_1 = to_fixed_32(p1);
    cout << endl << "Estimador p1: "     << p1 << endl;
    std::memcpy(mem + p1_Addr, param_1, 4);

    param_2 = to_fixed_32(p2);
    cout << "Estimador p2: "     << p2 << endl;
    std::memcpy(mem + p2_Addr, param_1, 4);

    volt = to_fixed_32(V);
    cout << "Estimador V : "     << V << endl;

    current = to_fixed_32(I);
    cout << "Estimador I : "     << I << endl << endl;

    done_IP.notify();
}

//Funcion TB
void  Device::tb(){
    wait(tb_do_event);

    // Open VCD file
    //sc_trace_file *wf = sc_create_vcd_trace_file("/estimador");
    //wf->set_time_unit(10, SC_NS);

    // Dump the desired signals
    //sc_trace(wf, adc_v, "adc_v");
    //sc_trace(wf, adc_i, "adc_i");
    //sc_trace(wf, start, "start");
    //sc_trace(wf, param_1, "param_1");
    //sc_trace(wf, param_2, "param_2");
    //sc_trace(wf, volt, "volt");
    //sc_trace(wf, current, "current");
    //*/

    for (int n=0; n < 1; n++){

        V_TB = InputVoltage(t)/22;
        I_TB = InputCurrent(t)/5;

        adc_v = to_fixed_16(V_TB);
        adc_i = to_fixed_16(I_TB);

        calc_t.notify();
        cout << endl << "Estimador " << "@" << sc_time_stamp()<< endl;

        wait(done_IP);
        wait(1,SC_NS);

        //-------------------------------
        t = t + step;
    }

    cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;

    //Close files
    //sc_close_vcd_trace_file(wf);
}
