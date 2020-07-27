#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "systemc.h"


#define calc_delay 0
//Constans from memory

#define I_scale_factor    1//5
#define V_scale_factor    1//22
#define Ig                3.99
#define GAMMA11           0.1
#define GAMMA12           0
#define GAMMA21           0
#define GAMMA22           100
#define INIT_ALPHA        0.55
#define INIT_BETA         -13.0
#define T_SAMPLING        1e-6

/*
float I_scale_factor=5;
float V_scale_factor=22;
float Ig=3.99;
float GAMMA11=0.1;
float GAMMA12=0;
float GAMMA21=0;
float GAMMA22=100;
float INIT_ALPHA = 0.55;
float INIT_BETA = -13.0;
float T_SAMPLING = 1e-3;
*/
SC_MODULE (estimador) {
    
  //-----------IP Ports----------------------------------
  sc_in < float >        adc_v; // 16 bit vector data from XADC
  sc_in < float >        adc_i; // 16 bit vector data from XADC
  //sc_in < sc_uint < 4 > >         tid;    // 4 bit vector channel ID from XADC
  //sc_in < bool >                  valid;      // active high, valid data from XADC
  //sc_out < bool >                 ready;      // Active high, ready signal from estimador
  sc_out < bool >        start;      // Active high, ready signal from estimador
  sc_out < float >       param_1; // 32 bit vector output of the estimador
  sc_out < float >       param_2; // 32 bit vector output of the estimador
  sc_out < float >       volt;
  sc_out < float >       current;
  //-----------Internal variables------------------------
  sc_event calc_t;

  float param_out_v[2]={0,0};
  float init_values[2]={INIT_ALPHA, INIT_BETA};
  float init_cond_1 = 0;
  float init_cond_2 = 0; 
  float p1, p2, p1_aux, p2_aux, y, I, V;


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
      //cout<<"start = "<< start <<endl<<endl;

      if(start){
        //cout<<"test"<<endl<<endl;
        init_cond_1 = INIT_ALPHA;
        init_cond_2 = INIT_BETA;
        
        cout<<"init_cond_1 = "<< init_cond_1 <<endl;
        cout<<"init_cond_2 = "<< init_cond_2 <<endl<<endl;
      };

      I = adc_i * I_scale_factor;
      V = adc_v * V_scale_factor;
      //#print(f'I esc = {I}  V esc = {V}')
      y= log(Ig - I);
      //#print(f'ln(y-ipv) {y}')
      p1=((GAMMA11*V+GAMMA12)*(y-(V*init_cond_1)-init_cond_2))*T_SAMPLING+init_cond_1;
      p2=((GAMMA21*V+GAMMA22)*(y-(V*init_cond_1)-init_cond_2))*T_SAMPLING+init_cond_2;
      //print(f' theta 1 = {theta1}  theta 2 = {theta2}\n\n')
      init_cond_1=p1;
      init_cond_2=p2;

      cout<<"param_1 = "<< p1 <<"   param_2 = "<< p2 <<endl<<endl;

      param_1 = p1;
      param_2 = p2;
      volt = adc_v;
      current = adc_i;

    }

  }


}; // End of Module estimador

