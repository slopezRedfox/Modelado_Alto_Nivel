//-----------------------------------------------------
#include "systemc.h"

SC_MODULE (ram) {
  
  //Se crean las variables data y address, estos son intiger sin signo de 32 bits
  //
  sc_inout<sc_uint<32> > data; 
  sc_in<sc_uint<32> > address;
 
  //-----------Internal variables-------------------
  //Se crea la memoria que es un array de 1024 intigers sin signo de 32 bits.
  //Se declaran las variables de evento (banderas) que indican lectura y escritura de la 
  //memoria
  sc_uint <32> mem[1024];
  sc_event rd_t, wr_t;
  
  //Se inicializa el constructor, 
  //
  // Constructor for memory
  //SC_CTOR(ram) {
  SC_HAS_PROCESS(ram);
    ram(sc_module_name ram) {
    SC_THREAD(wr);
    SC_THREAD(rd);
         
  } // End of Constructor

   //------------Code Starts Here-------------------------
  void write() {
    wr_t.notify(2, SC_NS);
  }  
  
  void read() {
    rd_t.notify(1, SC_NS);  
  }  

  void wr() {
    while(true) {
      wait(wr_t);
      mem [address.read()] = data.read();
    }  
  }

  void rd() {
    while(true) {
      wait(rd_t);
      data = mem [address.read()];
    }  
  }
  
}; // End of Module memory
