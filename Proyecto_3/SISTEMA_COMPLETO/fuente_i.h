#ifndef I_H
#define I_H

#include <systemc-ams>

SCA_TDF_MODULE (i_sig) {
    sca_tdf::sca_out<double> out; // output port

    i_sig(  sc_core::sc_module_name nm,
            sca_core::sca_time Tm_ = sca_core::sca_time(0.125, sc_core::SC_MS))
        
        : out("out"), Tm(Tm_) {}
    
    void set_attributes() {
        set_timestep(Tm);
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
private:
    sca_core::sca_time Tm; // module time step
};

#endif