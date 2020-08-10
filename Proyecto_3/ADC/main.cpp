#include <systemc-ams>
#include "adc.h"
#include "sin_src.h"
#include "constant_voltage_tdf.h"

int sc_main (int argc, char* argv[])
{
    uint32_t i;
    sca_tdf::sca_signal<double> In;
    sca_tdf::sca_signal<double> Rf;

    sc_core::sc_signal<bool> bit0;
    sc_core::sc_signal<bool> bit1;

    sc_core::sc_time time_step(10.0, sc_core::SC_NS);

    ADC comp0("comp0");
    comp0.In(In);        //Entrada
    comp0.Rf(Rf);        //Vmax del ADC
    comp0.bit0(bit0);    //Salida bit 0
    comp0.bit1(bit1);    //Salida bit 1

    sin_src sin0("sin0",3.3,10000,time_step);
    sin0.out(In);

    constant_voltage_tdf ref("ref",3.3);
    ref.out(Rf);

    sca_util::sca_trace_file *vcdfile= sca_util::sca_create_vcd_trace_file("output.vcd");
    sca_trace(vcdfile, In, "In");
    sca_trace(vcdfile, Rf, "Rf");
    sca_trace(vcdfile, bit0, "bit0");
    sca_trace(vcdfile, bit1, "bit1");

    sc_start(5, sc_core::SC_MS);

    sca_util::sca_close_vcd_trace_file(vcdfile);

	return 0;
}
