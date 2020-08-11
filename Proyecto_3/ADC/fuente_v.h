#ifndef V_H
#define V_H

#include <systemc-ams>

SCA_TDF_MODULE (v_sig) {
    sca_tdf::sca_out<double> out; // output port

    v_sig(  sc_core::sc_module_name nm,
            sca_core::sca_time Tm_ = sca_core::sca_time(0.125, sc_core::SC_MS))

        : out("out"), Tm(Tm_) {}
    
    void set_attributes() {
        set_timestep(Tm);
    }
    
    void processing() {
        double t     = get_time().to_seconds(); // actual time
        float  V_cte = 16.69;
        out.write((V_cte + (0.3 * V_cte * sin(2 * M_PI * 1000 * t))));
        //out.write((20 + (20 * sin(2 * M_PI * 1000 * t))));
    }
private:
    sca_core::sca_time Tm; // module time step
};

#endif