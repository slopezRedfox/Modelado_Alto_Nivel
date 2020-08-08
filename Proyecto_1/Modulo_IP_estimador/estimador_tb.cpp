#include <systemc.h>
#include "to_fixed.h"
#include "estimador.h"
#include "panel_model.h"


float t = 0;
float segundos=5;
float sample_rate=1e6;
float step=1/sample_rate;
float n_samples=segundos*sample_rate;
float V_TB, I_TB, I_float, V_float, P1_float, P2_float;
int I_int, V_int, P1_int, P2_int;


int sc_main (int argc, char* argv[]) {

  sc_signal < int > addr;
  sc_signal < float > data;
  
  sc_signal< sc_uint<16> > adc_v;
  sc_signal< sc_uint<16> > adc_i;
  
  sc_signal<bool>   start;
  sc_signal< sc_uint<32> > param_1;
  sc_signal< sc_uint<32> > param_2;
  sc_signal< sc_uint<32> > volt;
  sc_signal< sc_uint<32> > current;
  
  
  estimador estimador_dut("ESTIMADOR");

    estimador_dut.addr(addr);
    estimador_dut.data(data);
    estimador_dut.adc_v(adc_v);
    estimador_dut.adc_i(adc_i);
    estimador_dut.start(start);
    estimador_dut.param_1(param_1);
    estimador_dut.param_2(param_2);
    estimador_dut.volt(volt);
    estimador_dut.current(current);
    

  // Open VCD file
  sc_trace_file *wf = sc_create_vcd_trace_file("estimador");
  wf->set_time_unit(1, SC_NS);

  //open CSV file
  std::ofstream file_Signals;
  file_Signals.open ("SIGNALS.CSV");
  std::ofstream file_Params;
  file_Params.open ("PARAMS.CSV");
  
  // Dump the desired signals
  sc_trace(wf, adc_v, "adc_v");
  sc_trace(wf, adc_i, "adc_i");
  sc_trace(wf, start, "start");
  sc_trace(wf, param_1, "param_1");
  sc_trace(wf, param_2, "param_2");
  sc_trace(wf, volt, "volt");
  sc_trace(wf, current, "current");

  //Inicio del test
  

  sc_start(0,SC_NS);
  cout << "@" << sc_time_stamp()<< endl;
  start = 1;
  adc_v = 0;
  adc_i = 0;
  estimador_dut.process_sample();
  sc_start(50,SC_NS);
  printf("first\n");

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c00020; 
  data = 5;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c00024; 
  data = 22;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c00028; 
  data = 3.99;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c0002c; 
  data = 0.1;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c00030; 
  data = 0;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c00038; 
  data = 0;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c00040; 
  data = 100;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c00048; 
  data = 0.55;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c00050; 
  data = -13.0;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c00058; 
  data = 1e-6;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  addr = 0x43c00060; 
  data = 5;
  estimador_dut.set_up();
  sc_start(50,SC_NS);

  sc_start(0,SC_NS);
  cout << "@" << sc_time_stamp()<< endl;
  start = 1;
  //adc_v = 0;
  //adc_i = 0;
  estimador_dut.process_sample();
  sc_start(50,SC_NS);
  printf("first\n");

  cout << "step= " << step << endl;
  
  for(int i = 0; i < n_samples ; i++){  

    sc_start(50,SC_NS);
    start = 0;
    V_TB = InputVoltage(t)/22;
    I_TB = InputCurrent(t)/5;

    adc_v = to_fixed_16(V_TB);
    adc_i = to_fixed_16(I_TB);

    I_int = (int) current.read();
    I_float = (float) I_int / pow(2,21);

    V_int = (int) volt.read();
    V_float = (float) V_int / pow(2,21);

    P1_int = (int) param_1.read();
    P1_float = (float) P1_int / pow(2,21);

    P2_int = (int) param_2.read();
    P2_float = (float) P2_int / pow(2,21);



    file_Signals << t <<","<< I_float << ","<< V_float << endl;
    file_Params << t << ","<< P1_float << ","<< P2_float << endl;
    t = t + step;

    cout << "@" << sc_time_stamp()<< endl;
    estimador_dut.process_sample();
    cout<< "iter = "<<i<<endl;
    //sc_start(50,SC_NS);
  }
  


  sc_start(50,SC_NS); 

  cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
  
  //Close files
  sc_close_vcd_trace_file(wf);
  file_Signals.close();
  file_Params.close();
  return 0;// Terminate simulation

 }
