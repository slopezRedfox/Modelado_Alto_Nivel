#include <systemc.h>
#include "memory.cpp"

int sc_main (int argc, char* argv[]) {
 
  int data[8];
  //int dat[8];
  int address;
  
  ram mem("MEM");
            
  // Open VCD file
  sc_trace_file *wf = sc_create_vcd_trace_file("memory");
  wf->set_time_unit(1, SC_NS);
  
  // Dump the desired signals
  sc_trace(wf, data, "data");
  sc_trace(wf, address, "Address");
  
  sc_start(0,SC_NS);
  cout << "@" << sc_time_stamp()<< endl;

  // Data to be write into memory
  data[0] = 0xaced;
  data[1] = 0xbeef;
  data[2] = 0xdead;
  data[3] = 0x1234;
  data[4] = 0x1234;
  data[5] = 0xdead;
  data[6] = 0xbeef;
  data[7] = 0xaced;

  // Start the write proccess
  printf("\nWriting a sigle data\n");
  printf("WR: addr = 0x10, data = 0xaced\n");
  sc_start(1,SC_NS);
  address = 0x10;
  mem.write(address, data, 0);

  printf("\nWriting a 4-burst data\n");
  printf("WR: addr = 0x40, data = 0xaced\n");
  printf("WR: addr = 0x42, data = 0xbeef\n");
  printf("WR: addr = 0x43, data = 0xdead\n");
  printf("WR: addr = 0x44, data = 0x1234\n");
  sc_start(1,SC_NS);
  mem.write(0x40, data, 1);

  printf("\nWriting a 8-burst data\n");
  printf("WR: addr = 0x80, data = 0xaced\n");
  printf("WR: addr = 0x81, data = 0xbeef\n");
  printf("WR: addr = 0x82, data = 0xdead\n");
  printf("WR: addr = 0x83, data = 0x1234\n");
  printf("WR: addr = 0x84, data = 0x1234\n");
  printf("WR: addr = 0x85, data = 0xdead\n");
  printf("WR: addr = 0x86, data = 0xbeef\n");
  printf("WR: addr = 0x87, data = 0xaced\n");
  sc_start(1,SC_NS);
  mem.write(0x80, data, 2);

  // Start the read proccess
  printf("\nReading single data from memory address 0x10\n");
  sc_start(1,SC_NS);
  mem.read(0x10, data, 0);
  printf("RD: addr = 0x10, data = %x\n", data[0]);

  printf("\nReading 4-burst data from memory address 0x40\n");
  sc_start(1,SC_NS);
  mem.read(0x40, data, 1);
  printf("RD: addr = 0x40, data = %x\n", data[0]);
  printf("RD: addr = 0x41, data = %x\n", data[1]);
  printf("RD: addr = 0x42, data = %x\n", data[2]);
  printf("RD: addr = 0x43, data = %x\n", data[3]);
  
  printf("\nWriting a 8-burst data\n");
  sc_start(1,SC_NS);
  mem.read(0x80, data, 2);
  printf("WR: addr = 0x80, data = %x\n", data[0]);
  printf("WR: addr = 0x81, data = %x\n", data[1]);
  printf("WR: addr = 0x82, data = %x\n", data[2]);
  printf("WR: addr = 0x83, data = %x\n", data[3]);
  printf("WR: addr = 0x84, data = %x\n", data[4]);
  printf("WR: addr = 0x85, data = %x\n", data[5]);
  printf("WR: addr = 0x86, data = %x\n", data[6]);
  printf("WR: addr = 0x87, data = %x\n", data[7]);
  
  cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
  sc_close_vcd_trace_file(wf);
  return 0;// Terminate simulation

 }
