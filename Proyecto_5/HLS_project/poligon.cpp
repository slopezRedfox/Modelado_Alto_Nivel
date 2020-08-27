#include "poligon.hpp"

// THIS IS THE TOP LEVEL DESIGN THAT WILL BE SYNTHESIZED
int poligon_filling(int pixel_x, int pixel_y, int result, int in_x, int in_y, int in_value, int out_x, int out_y, int out_value){

#pragma HLS dataflow

	input_matrix <int> (in_x, in_y, in_value);

	result = InOut_Test <int> (pixel_x, pixel_y);

	Scan_Line <int> ();

	out_value = output_matrix <int> (out_x, out_y);

	return 0;

}

int wrapper_poligon(int pixel_x, int pixel_y, int result, int in_x, int in_y, int in_value, int out_x, int out_y, int out_value){

#pragma HLS INTERFACE ap_none port=pixel_x
#pragma HLS INTERFACE ap_none port=pixel_y
#pragma HLS INTERFACE ap_none port=in_x
#pragma HLS INTERFACE ap_none port=in_y
#pragma HLS INTERFACE ap_none port=in_value
#pragma HLS INTERFACE ap_none port=out_x
#pragma HLS INTERFACE ap_none port=out_y
#pragma HLS INTERFACE ap_none port=out_value
#pragma HLS INTERFACE ap_none port=result

#pragma HLS dataflow

	poligon_filling(pixel_x, pixel_y, result, in_x, in_y, in_value, out_x, out_y, out_value);
	return 0;
}


