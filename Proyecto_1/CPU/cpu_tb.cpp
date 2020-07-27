#include <systemc.h>
#include "cpu.cpp"

int sc_main (int argc, char* argv[]){
    int Data = 0;
    int Addr = 0;
    bool Rd_Wr = false;
    cpu cpu("cpu");

    sc_trace_file *wf = sc_create_vcd_trace_file("cpu");
    wf->set_time_unit(1, SC_NS);

    sc_trace(wf, Data, "Data");
    sc_trace(wf, Addr, "Addr");
    sc_trace(wf, Rd_Wr, "Rd_Wr");

    //sc_start(1,SC_NS);
    cout << "@" << sc_time_stamp() << endl;

    cout << "\nInicializador del estimador \n";
    sc_start(0,SC_NS);

    for (int i = 0; i < 11; i++)
    {
        Data = cpu.write_data(i);
        Addr = cpu.write_Addr(i);
        Rd_Wr = true;

        printf("Data = %x \t Address = %x \n",Data, Addr);

        sc_start(1,SC_NS);
    }

    cout << "\nLectura de registro del estimador \n";

    for (int i = 0; i < 4; i++)
    {
        Addr = cpu.read_addr(i);
        Data = 0xabcdabcd;
        Rd_Wr = false;
        sc_start(1,SC_NS);
        printf("Data = %x \t Address = %x \n",Data, Addr);
    }    

    cout << endl <<"@" << sc_time_stamp() << "Terminando simulacion.\n" << endl;

    sc_close_vcd_trace_file(wf);
    return 0;

    
}