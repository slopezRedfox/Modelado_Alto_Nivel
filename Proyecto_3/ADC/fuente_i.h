#ifndef I_H
#define I_H

#include <systemc-ams>

SCA_TDF_MODULE (i_sig) {
    sca_tdf::sca_out<double> out; // output port

    i_sig( sc_core::sc_module_name nm ) : out("out") {}
    
    void set_attributes() {
    }
    
    void processing() {
        double t     = get_time().to_seconds(); // actual time
        float  V_cte = 16.69;
        float Lambda = 3.99;
        float Psi = 5.1387085e-6; 
        float alpha = 0.625;
        float b = log(Psi);

        float V = V_cte + (0.3 * V_cte * sin(2 * M_PI * 1000 * t));

        out.write(Lambda - exp( alpha * V + b));
    }
};

#endif