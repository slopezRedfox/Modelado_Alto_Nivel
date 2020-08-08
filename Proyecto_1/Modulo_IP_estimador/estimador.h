#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "systemc.h"


#define delay 0
//Constans from memory
/*
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
*/
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
#define Set_flag_Addr       0x43c00060


SC_MODULE (estimador) {
    
  //-----------IP Ports----------------------------------
  //int start;
  sc_in < int > addr;
  sc_in < float > data;

  sc_in < sc_uint<16> >        adc_v; // vector data from XADC
  sc_in < sc_uint<16> >        adc_i; // vector data from XADC

  sc_out < bool >        start;      // Active high, ready signal from estimador
  sc_out < sc_uint<32> >       param_1; // 32 bit vector output of the estimador
  sc_out < sc_uint<32> >       param_2; // 32 bit vector output of the estimador
  sc_out < sc_uint<32> >       volt;
  sc_out < sc_uint<32> >       current;
  //-----------Internal variables------------------------
  sc_event calc_t, setup_t;

  float init_cond_1, init_cond_2; 
  float p1, p2, p1_aux, p2_aux, y, I, V;
  int I_int_est, V_int_est;

  float I_scale_factor, V_scale_factor, Ig, GAMMA11, GAMMA12, GAMMA21, GAMMA22, INIT_ALPHA, INIT_BETA, T_SAMPLING;

  // Constructor for estimador
  SC_HAS_PROCESS(estimador);
    estimador(sc_module_name estimador) {
    SC_THREAD(estimador_main);
    SC_THREAD(set_up_estimador);
  }

   //------------Code Starts Here-------------------------

  //Datos listos
  void process_sample() {
    calc_t.notify(delay, SC_NS);
  }
  void set_up() {
    setup_t.notify(delay, SC_NS);
  }

  void set_up_estimador(){
  	while (true){

  		wait(setup_t);

	  		if(addr== I_scale_factor_Addr){
	  			I_scale_factor = data;
	  			cout << "I scale" << I_scale_factor << endl;
		  	}
		  	else if(addr == V_scale_factor_Addr){
		  		V_scale_factor = data;
		  		cout << "I scale" << I_scale_factor << endl;
		  	}
		  	else if(addr == V_scale_factor_Addr){
		  		V_scale_factor = data;
		  		cout << "V_scale_factor" << V_scale_factor << endl;
		  	}
		  	else if(addr == Ig_value_Addr){
		  		Ig = data;
		  		cout << "Ig" << Ig << endl;
		  	}
		  	else if(addr == Gamma11_Addr){
		  		GAMMA11 = data;
		  		cout << "GAMMA11" << GAMMA11 << endl;
		  	}
		  	else if(addr == Gamma12_Addr){
		  		GAMMA12 = data;
		  		cout << "GAMMA12" << GAMMA12 << endl;
		  	}
		  	else if(addr == Gamma21_Addr){
		  		GAMMA21 = data;
		  		cout << "GAMMA21" << GAMMA21 << endl;
		  	}
		  	else if(addr == Gamma22_Addr){
		  		GAMMA22 = data;
		  		cout << "GAMMA22" << GAMMA22 << endl;
		  	}
		  	else if(addr == Init_alpha_Addr){
		  		INIT_ALPHA = data;
		  		cout << "INIT_ALPHA" << INIT_ALPHA << endl;
		  	}
		  	else if(addr == Init_beta_Addr){
		  		INIT_BETA = data;
		  		cout << "INIT_BETA" << INIT_BETA << endl;
		  	}
		  	else if(addr == T_sampling_Addr){
		  		T_SAMPLING = data;
		  		cout << "T_SAMPLING" << T_SAMPLING << endl;
		  	}
  	}
  	
  }

  void estimador_main(){

    while(true){

      wait(calc_t);

      if(start){
        init_cond_1 = INIT_ALPHA;
        init_cond_2 = INIT_BETA;
        start=0;
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

