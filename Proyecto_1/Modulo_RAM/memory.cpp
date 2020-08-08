//-----------------------------------------------------
#include "systemc.h"

#define SIZE        512000000
#define WR_DELAY           10
#define RD_DELAY            5
#define BURST_DELAY         1


SC_MODULE (ram) {
 
  //-----------Internal variables-------------------
  int * mem;
  int data;
  int address;
  int burst;
  int count;
  sc_event rd_t, wr_t;
  
  // Constructor for memory
  //SC_CTOR(ram) {
  SC_HAS_PROCESS(ram);
    ram(sc_module_name ram) {
    mem = new int [SIZE];
    count = 0x0;
    SC_THREAD(wr);
    //SC_THREAD(rd);
         
  } // End of Constructor

   //------------Code Starts Here-------------------------
  void write(int addr, int dat, int burst) {
    //printf("\nCount = %d\n", count);
    if(burst == 0){
      address = addr;
    }else if(burst == 1||burst == 2){
      address = addr + count;
      count++;
    }
    data = dat;
    if((burst==1&&count==4)||(burst==2&&count==8)){
      count = 0;
    }
    printf("WD: mem[0x%x] = 0x%x\n", address, data);
    wr_t.notify(0, SC_NS);
  }

  void wr(){
    while(true){
      wait(wr_t);
      mem[address] = data;
    }
  }

  int read(int addr, int burst){
    int data=0;
    if(burst == 0){
      address = addr;
    }else if(burst == 1||burst == 2){
      address = addr + count;
      count++;
    }
    if((burst==1&&count==4)||(burst==2&&count==8)){
      count = 0;
    }
    data = mem[address];
    printf("RD: mem[0x%x] = 0x%x\n", address, data);
    return data;
  }

}; // End of Module memory
