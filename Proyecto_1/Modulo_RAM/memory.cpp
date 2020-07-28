//-----------------------------------------------------
#include "systemc.h"

#define WR_DELAY     10
#define RD_DELAY      5
#define BURST_DELAY   1

SC_MODULE (ram) {
 
  //-----------Internal variables-------------------
  int * mem;
  int * data;
  int address;
  int burst;
  sc_event rd_t, wr_t;
  
  // Constructor for memory
  //SC_CTOR(ram) {
  SC_HAS_PROCESS(ram);
    ram(sc_module_name ram, int size=512e6) {
    mem = new int [size];
    SC_THREAD(wr);
    SC_THREAD(rd);
         
  } // End of Constructor

   //------------Code Starts Here-------------------------
  void write(int addr, int * dat, int brst=0) {
    data = dat;
    address = addr;
    burst = brst;
    wr_t.notify(WR_DELAY, SC_NS);
  }  

  void read(int addr, int * dat, int brst=0) {
    data = dat;
    address = addr;
    burst = brst;
    rd_t.notify(RD_DELAY, SC_NS);
  }  

  void rd() 
  {
    while (true)
    {
      if(burst == 0)
      {
        data[0] = mem[addr];
      } 
      else if(burst == 1)
      {
        for(int i; i < 4; i++)
        {
          data[i] = mem[addr + i];
          wait(BURST_DELAY, SC_NS);
        }
      } 
      else if (burst == 2)
      {
        for(int i; i < 8; i++)
        {
          data[i] = mem[addr + i];
          wait(BURST_DELAY, SC_NS);
        }
      }
    }
  }  

  void wr() {
    while(true) {
      wait(wr_t);
      if (burst == 0)
      {
        mem [address] = data;
      }
      else if (burst == 1)
      {
        for (int i = 0; i < 4; i++)
        {
          mem[address+i] = data[i];
          wait(BURST_DELAY, SC_NS);
        }
        
      }
      else if (burst == 2)
      {
        for (int i = 0; i < 8; i++)
        {
          mem[address+i] = data[i];
          wait(BURST_DELAY, SC_NS);
        } 
      }
    }  
  }

  
}; // End of Module memory
