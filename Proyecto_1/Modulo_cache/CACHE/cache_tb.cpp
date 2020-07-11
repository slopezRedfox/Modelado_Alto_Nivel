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
  sc_trace(wf, cacheL2.Aux  , "Fun");        
  sc_trace(wf, cacheL2.tag  , "address_tag");



  //way 0, offset 0, set 0
  sc_trace(wf, cacheL2.mem.sets[0].block[0].tag[0] , "0_tag" );
  sc_trace(wf, cacheL2.mem.sets[0].block[0].data[0], "0_data");
  sc_trace(wf, cacheL2.mem.sets[0].block[0].use[0] , "0_use" );
  
  //way 1, offset 0, set 0
  sc_trace(wf, cacheL2.mem.sets[0].block[1].tag[0] , "1_tag" );
  sc_trace(wf, cacheL2.mem.sets[0].block[1].data[0], "1_data");
  sc_trace(wf, cacheL2.mem.sets[0].block[1].use[0] , "1_use" );

  //way 2, offset 0, set 0
  sc_trace(wf, cacheL2.mem.sets[0].block[2].tag[0] , "2_tag" );
  sc_trace(wf, cacheL2.mem.sets[0].block[2].data[0], "2_data");
  sc_trace(wf, cacheL2.mem.sets[0].block[2].use[0] , "2_use" );
  
  //way 3, offset 0, set 0
  sc_trace(wf, cacheL2.mem.sets[0].block[3].tag[0] , "3_tag" );
  sc_trace(wf, cacheL2.mem.sets[0].block[3].data[0], "3_data");
  sc_trace(wf, cacheL2.mem.sets[0].block[3].use[0] , "3_use" );
 /* 
  //way 4, offset 0, set 0
  sc_trace(wf, cacheL2.mem.sets[0].block[4].tag[0] , "4_tag" );
  sc_trace(wf, cacheL2.mem.sets[0].block[4].data[0], "4_data");
  sc_trace(wf, cacheL2.mem.sets[0].block[4].use[0] , "4_use" );
  
  //way 5, offset 0, set 0
  sc_trace(wf, cacheL2.mem.sets[0].block[5].tag[0] , "5_tag" );
  sc_trace(wf, cacheL2.mem.sets[0].block[5].data[0], "5_data");
  sc_trace(wf, cacheL2.mem.sets[0].block[5].use[0] , "5_use" );
  
  //way 6, offset 0, set 0
  sc_trace(wf, cacheL2.mem.sets[0].block[6].tag[0] , "6_tag" );
  sc_trace(wf, cacheL2.mem.sets[0].block[6].data[0], "6_data");
  sc_trace(wf, cacheL2.mem.sets[0].block[6].use[0] , "6_use" );
  
  //way 7, offset 0, set 0
  sc_trace(wf, cacheL2.mem.sets[0].block[7].tag[0] , "7_tag" );
  sc_trace(wf, cacheL2.mem.sets[0].block[7].data[0], "7_data");
  sc_trace(wf, cacheL2.mem.sets[0].block[7].use[0] , "7_use" );
*/
  sc_trace(wf, TestClk, "zclk");
  
  //Inicio de prueba
  sc_start(0,SC_NS);
  cout << "@" << sc_time_stamp()<< endl;
  
  printf("Wr: 0x1\n");
  on  = 1;
  r_w = 0;
        //                              offset
        // |      tag        |  index    | |
  address=0b00000000000000000100000000000000;
  data   =0x0A;
  cacheL2.write();
  sc_start(50,SC_NS);

  printf("Wr: 0x3\n");
  on  = 1;
  r_w = 0;
        //                              offset
        // |      tag        |  index    | |
  address=0b00000000000000001100000000000000;
  data   =0x0B;
  cacheL2.write();
  sc_start(50,SC_NS);

  printf("Wr: 0x7\n");
  on  = 1;
  r_w = 0;
        //                              offset
        // |      tag        |  index    | |
  address=0b00000000000000011100000000000000;
  data   =0x0C;
  cacheL2.write();
  sc_start(50,SC_NS);

  printf("Wr: 0xF\n");
  on  = 1;
  r_w = 0;
        //                              offset
        // |      tag        |  index    | |
  address=0b00000000000000111100000000000000;
  data   =0x0D;
  cacheL2.write();
  sc_start(50,SC_NS);

  printf("Wr: 0x1F\n");
  on  = 1;
  r_w = 0;
        //                              offset
        // |      tag        |  index    | |
  address=0b00000000000001111100000000000000;
  data   =0x0E;
  cacheL2.write();
  sc_start(50,SC_NS);
/*
  printf("Wr: 0x3F\n");
  on  = 1;
  r_w = 0;
        //                              offset
        // |      tag        |  index    | |
  address=0b00000000000011111100000000000000;
  data   =0x0F;
  cacheL2.write();
  sc_start(50,SC_NS);

  printf("Wr: 0x7F\n");
  on  = 1;
  r_w = 0;
        //                              offset
        // |      tag        |  index    | |
  address=0b00000000000111111100000000000000;
  data   =0xAF;
  cacheL2.write();
  sc_start(50,SC_NS);

  printf("Wr: 0xFF\n");
  on  = 1;
  r_w = 0;
        //                              offset
        // |      tag        |  index    | |
  address=0b00000000001111111100000000000000;
  data   =0xBF;
  cacheL2.write();
  sc_start(50,SC_NS);

  printf("Wr: 0x1FF\n");
  on  = 1;
  r_w = 0;
        //                              offset
        // |      tag        |  index    | |
  address=0b00000000011111111100000000000000;
  data   =0xCF;
  cacheL2.write();
  sc_start(50,SC_NS);
*/
  printf("Rd: 0x7\n");
  on  = 1;
  r_w = 1;
        //                              offset
        // |      tag        |  index    | |
  address=0b00000000000000011100000000000000;
  data   =0x00;
  cacheL2.read();
  sc_start(50,SC_NS);

  printf("Rd: 0x7\n");
  on  = 1;
  r_w = 1;
        //                              offset
        // |      tag        |  index    | |
  address=0b00001000000000011100000000000000;
  data   =0x00;
  cacheL2.read();
  sc_start(50,SC_NS);

  //FIN DE ESCRITURA
  cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
  sc_close_vcd_trace_file(wf);
  return 0;// Terminate simulation

 }
