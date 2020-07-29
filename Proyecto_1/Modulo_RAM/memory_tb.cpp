#include <systemc.h>
#include "memory.cpp"

int sc_main (int argc, char* argv[]) {
 
  int data;
  int address;
  
  ram mem("MEM");
            
  // Open VCD file
  sc_trace_file *wf = sc_create_vcd_trace_file("memory");
  wf->set_time_unit(1, SC_NS);
  
  // Dump the desired signals
  sc_trace(wf, data, "data");
  
  sc_start(0, SC_NS);
  cout << "@" << sc_time_stamp()<< "Simulation started" << endl;

  // Start the write proccess
  printf("\nWriting a sigle data\n");
  address = 0x10;
  data = 0xaced;
  mem.write(address, data, 0);
  sc_start(15,SC_NS);
  //printf("WR: address = 0x%x, data = 0x%x\n",address,data);

  cout << "@" << sc_time_stamp()<< endl;
  printf("\nWriting a 4-burst data\n");
  address = 0x40;
  data = 0xaced;
  mem.write(address, data, 1);
  //printf("WR: address = 0x%x, data = 0x%x\n",address,data);
  sc_start(15,SC_NS);
  data = 0xbeef;
  mem.write(address, data, 1);
  //printf("WR: address = 0x%x, data = 0x%x\n",address+1,data);
  sc_start(5,SC_NS);
  data = 0xdead;
  mem.write(address, data, 1);
  //printf("WR: address = 0x%x, data = 0x%x\n",address+2,data);
  sc_start(5,SC_NS);
  data = 0x1234;
  mem.write(address, data, 1);
  //printf("WR: address = 0x%x, data = 0x%x\n",address+3,data);
  sc_start(5,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  printf("\nWriting a 8-burst data\n");
  address = 0x80;
  data = 0xaced;
  mem.write(address, data, 2);
  //printf("WR: address = 0x%x, data = 0x%x\n",address,data);
  sc_start(15,SC_NS);
  data = 0xbeef;
  mem.write(address, data, 2);
  //printf("WR: address = 0x%x, data = 0x%x\n",address,data);
  sc_start(5,SC_NS);
  data = 0xdead;
  mem.write(address, data, 2);
  //printf("WR: address = 0x%x, data = 0x%x\n",address,data);
  sc_start(5,SC_NS);
  data = 0x1234;
  mem.write(address, data, 2);
  //printf("WR: address = 0x%x, data = 0x%x\n",address,data);
  sc_start(5,SC_NS);
  data = 0xaced;
  mem.write(address, data, 2);
  //printf("WR: address = 0x%x, data = 0x%x\n",address,data);
  sc_start(5,SC_NS);
  data = 0xbeef;
  mem.write(address, data, 2);
  //printf("WR: address = 0x%x, data = 0x%x\n",address,data);
  sc_start(5,SC_NS);
  data = 0xdead;
  mem.write(address, data, 2);
  //printf("WR: address = 0x%x, data = 0x%x\n",address,data);
  sc_start(5,SC_NS);
  data = 0x1234;
  mem.write(address, data, 2);
  //printf("WR: address = 0x%x, data = 0x%x\n",address,data);
  sc_start(5,SC_NS);

  // Start the read proccess
  cout << "@" << sc_time_stamp()<< endl;
  printf("\nReading single data from memory address 0x10\n");
  data = mem.read(0x10, 0);
  //printf("RD: address = 0x10, data = 0x%x\n", data);
  sc_start(10,SC_NS);

  cout << "@" << sc_time_stamp()<< endl;
  printf("\nReading 4-burst data from memory address 0x40\n");
  address = 0x40;
  data = mem.read(address, 1);
  //printf("RD: address = 0x40, data = 0x%x\n", data);
  sc_start(10,SC_NS);
  data = mem.read(address, 1);
  //printf("RD: address = 0x41, data = 0x%x\n", data);
  sc_start(1,SC_NS);
  data = mem.read(address, 1);
  //printf("RD: address = 0x42, data = 0x%x\n", data);
  sc_start(1,SC_NS);
  data = mem.read(address, 1);
  //printf("RD: address = 0x43, data = 0x%x\n", data);
  sc_start(1,SC_NS);
  
  
  cout << "@" << sc_time_stamp()<< endl;
  printf("\nReading 8-burst data from memory address 0x40\n");
  address = 0x80;
  data = mem.read(address, 2);
  //printf("RD: address = 0x%x, data = 0x%x\n", address, data);
  sc_start(10,SC_NS);
  data = mem.read(address, 2);
  //printf("RD: address = 0x%x, data = 0x%x\n", address, data);
  sc_start(1,SC_NS);
  data = mem.read(address, 2);
  //printf("RD: address = 0x%x, data = 0x%x\n", address, data);
  sc_start(1,SC_NS);
  data = mem.read(address, 2);
  //printf("RD: address = 0x%x, data = 0x%x\n", address, data);
  sc_start(1,SC_NS);
  data = mem.read(address, 2);
  //printf("RD: address = 0x%x, data = 0x%x\n", address, data);
  sc_start(10,SC_NS);
  data = mem.read(address, 2);
  //printf("RD: address = 0x%x, data = 0x%x\n", address, data);
  sc_start(1,SC_NS);
  data = mem.read(address, 2);
  //printf("RD: address = 0x%x, data = 0x%x\n", address, data);
  sc_start(1,SC_NS);
  data = mem.read(address, 2);
  //printf("RD: address = 0x%x, data = 0x%x\n", address, data);
  sc_start(1,SC_NS);

  cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
  sc_close_vcd_trace_file(wf);
  return 0;// Terminate simulation

 }
