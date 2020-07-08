#include <systemc.h>
#include "cache.cpp"

int sc_main (int argc, char* argv[]) {
  //Inicializacion de señales
  //Banderas
  sc_signal<bool> on, rst;
  sc_signal<bool> r_w, flag;

  //Buses de datos
  sc_signal<sc_uint<32>> data; 
  sc_signal<sc_uint<32>> address;

  //Reloj del sistema a 10ns
  sc_clock TestClk("TestClock", 10, SC_NS);
  
  //Cableado del modulo
  cache cacheL2("NAND");
    cacheL2.on(on);
    cacheL2.rst(rst);
    cacheL2.r_w(r_w);
    cacheL2.flag(flag);

    cacheL2.address(address);
    cacheL2.data(data);

    cacheL2.clk(TestClk);

  //INICIO DE ESCRITURA
  sc_trace_file *wf = sc_create_vcd_trace_file("cache");
  wf->set_time_unit(1, SC_NS);
  
  // Señales a observar
  sc_trace(wf,     rst,     "rst");
  sc_trace(wf,    flag,    "flag");
  sc_trace(wf,    data,    "data");
  sc_trace(wf, address, "address");
  sc_trace(wf, cacheL2.Aux           , "Fun");  
  sc_trace(wf, cacheL2.mem[0].address, "0_MemAdd");
  sc_trace(wf, cacheL2.mem[0].data   , "0_MemDat");
  sc_trace(wf, cacheL2.mem[1].address, "1_MemAdd");
  sc_trace(wf, cacheL2.mem[1].data   , "1_MemDat");
  sc_trace(wf, cacheL2.mem[2].address, "2_MemAdd");
  sc_trace(wf, cacheL2.mem[2].data   , "2_MemDat");
  sc_trace(wf, TestClk,     "zclk");
  
  //Inicio de prueba
  sc_start(0,SC_NS);
  cout << "@" << sc_time_stamp()<< endl;
  
  printf("Wr: 0x3\n");
  on  = 1;
  r_w = 0;
  address=0x3;
  data=0x33;
  cacheL2.write();
  sc_start(20,SC_NS);

  printf("Wr: 0x2\n");
  on  = 1;
  r_w = 0;
  address=0x2;
  data=0x22;
  cacheL2.write();
  sc_start(20,SC_NS);

  printf("Wr: 0x5\n");
  on  = 1;
  r_w = 0;
  address=0x5;
  data=0x66;
  cacheL2.write();
  sc_start(20,SC_NS);

  printf("Rd: 0x2\n");
  on  = 1;
  r_w = 1;
  address=0x2;
  data=0x0;
  cacheL2.read();
  sc_start(20,SC_NS);

  printf("Rd: 0x1\n");
  on  = 1;
  r_w = 1;
  address=0x1;
  data=0x0;
  cacheL2.read();
  sc_start(20,SC_NS);

  printf("Wr: 0x1\n");
  on  = 1;
  r_w = 0;
  address=0x1;
  data=0x11;
  cacheL2.write();
  sc_start(20,SC_NS);

  printf("Rd: 0x5\n");
  on  = 1;
  r_w = 1;
  address=0x5;
  data=0x0;
  cacheL2.read();
  sc_start(20,SC_NS);

  printf("Wr: 0x5d\n");
  on  = 1;
  r_w = 0;
  address=0x5d;
  data=0xff;
  cacheL2.write();
  sc_start(20,SC_NS);

  printf("Rd: 0x1\n");
  on  = 1;
  r_w = 1;
  address=0x2;
  data=0x0;
  cacheL2.read();
  sc_start(20,SC_NS);

  //FIN DE ESCRITURA
  cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
  sc_close_vcd_trace_file(wf);
  return 0;// Terminate simulation

 }