#include <systemc.h>
#include "nand.cpp"

int sc_main (int argc, char* argv[]) {
  sc_signal<bool> ASig, BSig, FSig;
  sc_clock TestClk("TestClock", 10, SC_NS, 0.5);
  
  nand nand2("NAND");
  nand2.A(ASig);
  nand2.B(BSig);
  nand2.F(FSig);
  nand2.clk(TestClk); 
            
  // Open VCD file
  sc_trace_file *wf = sc_create_vcd_trace_file("nand");
  wf->set_time_unit(1, SC_NS);
  
  // Dump the desired signals
  sc_trace(wf, ASig, "A");
  sc_trace(wf, BSig, "B");
  sc_trace(wf, FSig, "F");
  sc_trace(wf, TestClk, "clk");  
  
  sc_start(0,SC_NS);
  cout << "@" << sc_time_stamp()<< endl;
  
  printf("A = 0 || B = 0\n");
  ASig=0;
  BSig=0;
  sc_start(20,SC_NS);

  printf("A = 0 || B = 1\n");
  ASig=0;
  BSig=1;
  sc_start(11,SC_NS);

  printf("A = 1 || B = 0\n");
  ASig=1;
  BSig=0;
  sc_start(20,SC_NS);

  printf("A = 1 || B = 1\n");
  ASig=1;
  BSig=1;
  sc_start(20,SC_NS);

  printf("A = 0 || B = 0\n");
  ASig=0;
  BSig=0;
  sc_start(20,SC_NS);

  printf("A = 0 || B = 1\n");
  ASig=0;
  BSig=1;
  sc_start(20,SC_NS);

  printf("A = 1 || B = 0\n");
  ASig=1;
  BSig=0;
  sc_start(20,SC_NS);

  printf("A = 1 || B = 1\n");
  ASig=1;
  BSig=1;
  sc_start(20,SC_NS);

  cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
  sc_close_vcd_trace_file(wf);
  return 0;// Terminate simulation

 }