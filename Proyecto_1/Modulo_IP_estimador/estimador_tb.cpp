#include <systemc.h>
#include "to_fixed.h"
#include "estimador.h"
#include "panel_model.h"


float t = 0;
float segundos=5;
float sample_rate=1e6;
float step=1/sample_rate;
float n_samples=segundos*sample_rate;
float V_TB, I_TB;


int sc_main (int argc, char* argv[]) {
  
  sc_signal< sc_uint<16> > adc_v;
  sc_signal< sc_uint<16> > adc_i;
  
  sc_signal<bool>   start;
  sc_signal< sc_uint<32> > param_1;
  sc_signal< sc_uint<32> > param_2;
  sc_signal< sc_uint<32> > volt;
  sc_signal< sc_uint<32> > current;
  
  
  estimador estimador_dut("ESTIMADOR");

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

  cout << "step= " << step << endl;
  
  for(int i = 0; i <n_samples; i++){  

    sc_start(50,SC_NS);
    start = 0;
    V_TB = InputVoltage(t)/22;
    I_TB = InputCurrent(t)/5;

    adc_v = to_fixed_16(V_TB);
    adc_i = to_fixed_16(I_TB);

    file_Signals << t <<","<< I_TB << ","<< V_TB << endl;
    file_Params << t << ","<< param_1 << ","<< param_2 << endl;
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
