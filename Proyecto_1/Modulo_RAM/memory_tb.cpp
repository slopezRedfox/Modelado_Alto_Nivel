#include <systemc.h>
#include "memory.cpp"

int sc_main (int argc, char* argv[]) {
 
  int* data;
  int dat[8];
  
  ram mem("MEM");
            
  // Open VCD file
  sc_trace_file *wf = sc_create_vcd_trace_file("memory");
  wf->set_time_unit(1, SC_NS);
  
  // Dump the desired signals
  sc_trace(wf, data, "data");
  
  sc_start(0,SC_NS);
  cout << "@" << sc_time_stamp()<< endl;

  // Data to be write into memory
  dat[0] = 0xaced;
  dat[1] = 0xbeef;
  dat[2] = 0xdead;
  dat[3] = 0x1234;
  dat[4] = 0x1234;
  dat[5] = 0xdead;
  dat[6] = 0xbeef;
  dat[7] = 0xaced;

  // Start the write proccess
  printf("\nWriting a sigle data\n");
  printf("WR: addr = 0x10, data = 0xaced\n");
  sc_start(1,SC_NS);
  mem.write(0x10, &dat[0], 0);

  /*
  printf("\nWriting a 4-burst data\n");
  printf("WR: addr = 0x40, data = 0xaced\n");
  printf("WR: addr = 0x42, data = 0xbeef\n");
  printf("WR: addr = 0x43, data = 0xdead\n");
  printf("WR: addr = 0x44, data = 0x1234\n");
  sc_start(1,SC_NS);
  mem.write(0x40, &dat[0], 1);

  printf("\nWriting a 8-burst data\n");
  printf("WR: addr = 0x80, data = 0xaced\n");
  printf("WR: addr = 0x81, data = 0xbeef\n");
  printf("WR: addr = 0x82, data = 0xdead\n");
  printf("WR: addr = 0x83, data = 0x1234\n");
  printf("WR: addr = 0x84, data = 0x1234\n");
  printf("WR: addr = 0x85, data = 0xdead\n");
  printf("WR: addr = 0x86, data = 0xbeef\n");
  printf("WR: addr = 0x87, data = 0xaced\n");
  sc_start(0,SC_NS);
  mem.write(0x80, &dat[0], 2);
  */

  // Start the read proccess
  printf("\nReading single data from memory address 0x10\n");
  mem.read(0x10, data, 0);
  printf("Data[0x10] = %x\n", data[0]);

  /*printf("\nReading 4-burst data from memory address 0x40\n");
  mem.read(0x40, data, 1);
  printf("Data[0x40] = %x\n", data[0]);
  printf("Data[0x41] = %x\n", data[1]);
  printf("Data[0x42] = %x\n", data[2]);
  printf("Data[0x43] = %x\n", data[3]);
  */


  cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
  sc_close_vcd_trace_file(wf);
  return 0;// Terminate simulation

 }
