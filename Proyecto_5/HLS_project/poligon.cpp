#include "poligon.hpp"

// THIS IS THE TOP LEVEL DESIGN THAT WILL BE SYNTHESIZED


int wrapper_poligon(){

#pragma HLS INTERFACE ap_ctrl_hs port=return
#pragma HLS INTERFACE axis register both port=seq_in_xadc
#pragma HLS INTERFACE s_axilite port=interface_param_apprx
#pragma HLS INTERFACE s_axilite register port=I_scale_factor
#pragma HLS INTERFACE s_axilite register port=V_scale_factor
#pragma HLS INTERFACE s_axilite register port=Ig

#pragma HLS dataflow


	return 0;
}


