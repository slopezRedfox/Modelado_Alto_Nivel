#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "systemc.h"


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
  int I_int_est, V_int_est;


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

      I_int_est = (int)adc_i.read();
      V_int_est = (int)adc_v.read();

      I = (float)I_int_est / pow(2,16);
      V = (float)V_int_est / pow(2,16);

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

