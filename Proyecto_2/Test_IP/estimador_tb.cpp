#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <iostream>
#include <fstream>


#include "systemc.h"

using namespace std;

#define calc_delay 0
//Constans from memory

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

float Lambda = 3.99;                     	//Short-Circuit current
float Psi = 5.1387085e-6;                	//Is current (saturation)
float alpha = 0.625;                 			//Thermal voltage relation
float V_oc = 1/alpha*(log(Lambda/Psi));   //Open circuit voltage
float V_mpp = 17.4;                  			//Maximum power point voltage
float I_mpp = 3.75;                  			//Maximum power point current
float P_mpp = 65.25;                 			//Maximum power 
float y = log(Lambda);              			//Short-Circuit logarithm
float b = log(Psi);                 			//Is current logarithm
float V_cte = 16.69;

float t = 0;
float segundos=5;
float sample_rate=1e6;
float step=1/sample_rate;
float n_samples=segundos*sample_rate;
float V_TB, I_TB;


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

SC_MODULE (estimador) {
    
  //-----------IP Ports----------------------------------
  sc_in < sc_uint<16> >        adc_v; // vector data from XADC
  sc_in < sc_uint<16> >        adc_i; // vector data from XADC

  sc_out < bool >        start;      // Active high, ready signal from estimador
  sc_out < sc_uint<32> >       param_1; // 32 bit vector output of the estimador
  sc_out < sc_uint<32> >       param_2; // 32 bit vector output of the estimador
  sc_out < sc_uint<32> >       volt;
  sc_out < sc_uint<32> >       current;
  //-----------Internal variables------------------------
  sc_event calc_t;

  float init_cond_1, init_cond_2; 
  float p1, p2, p1_aux, p2_aux, y, I, V;

  float H,L;

  // Constructor for estimador
  SC_HAS_PROCESS(estimador);
    estimador(sc_module_name estimador) {
    SC_THREAD(estimador_main);
  }

   //------------Code Starts Here-------------------------

  //Datos listos
  void process_sample() {
    calc_t.notify(calc_delay, SC_NS);
  }

  void estimador_main(){

    while(true){

      wait(calc_t);

      if(start){
        init_cond_1 = INIT_ALPHA;
        init_cond_2 = INIT_BETA;
      };

      I = adc_i.read() / pow(2,16);
      V = adc_v.read() / pow(2,16);

      I *= I_scale_factor;
      V *= V_scale_factor;
      y= log(Ig - I);
      p1=((GAMMA11*V+GAMMA12)*(y-(V*init_cond_1)-init_cond_2))*T_SAMPLING+init_cond_1;
      p2=((GAMMA21*V+GAMMA22)*(y-(V*init_cond_1)-init_cond_2))*T_SAMPLING+init_cond_2;
      init_cond_1=p1;
      init_cond_2=p2;

      cout<<"param_1 = "<< p1 <<"   param_2 = "<< p2 <<endl<<endl;

      param_1 = to_fixed_32(p1);
      param_2 = to_fixed_32(p2);
      volt = to_fixed_32(V);
      current = to_fixed_32(I);

    }

  }

}; // End of Module estimador

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
  
  for(int i = 0; i <100; i++){  

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
