//=========================================================
//Function: DDR3 DRAM
//Size: 256 MB
//Speed: 1066 Mbps
//I/O:   Address[14:0]       Bits de direccion y comando
//       Bank Address[2:0]   Bits de direccion de bloque
//       CKP                 Clock diferencial
//       CKN                 Clock diferencial
//       CKE                 Clock enable
//       CS                  Chip Select
//       DM                  Data mask
//       ODT                 On die termination
//       RAS                 Row Address strobe
//       CAS                 Column Address strobe
//       WE                  Write enable
//       RESET               Reset
//       DQ[15:0]            Data
//========================================================


#include "systemc.h"

#define DATA_WIDTH    32
#define ADDR_WIDTH    15
#define BLCK_WIDTH     3
#define ROW_SIZE   16384
#define COL_SIZE     128
#define rd_delay      10
#define wr_delay      10
#define burst_delay    1

// Una ram esta compuesta por dos memorias de tipo
// DDR3 de 128 Mbit x 16
class Dram{
  public:
    sc_uint <16> data[ROW_SIZE][COL_SIZE][BLCK_SIZE][2];
    bool bl = false;

SC_MODULE (dram) {

  // Variables internas
  Dram* mem;
  int data;
  int address;
  int b_address;
  int cas;
  int ras;
  sc_event wr_t;

  // Constructor
  SC_HAS_PROCESS(dram);
    dram(sc_module_name dram){
      dram = new Dram mem;
      SC_THREAD(wr);
      SC_THREAD(rd);
    }

  // Funciones
  void write(int addr, int blck, int data_in){
    data = data_in;
    address = addr;
    b_address = blck;
    wr_t.notify(wr_delay, SC_NS);
  }

  void rd(){
    while(true){
      wait(wr_t);
      // Solo se escribe 1 dato
      if(address[10] == 0){
        mem.data[address.range(14,0)][address.range(7,0)][b_address][0] = data.range(7,0);
        mem.data[address.range(14,0)][address.range(7,0)][b_address][1] = data.range(15,8);
      }
      // Se escriben 4 datos seguidos
      else if((ddress[10] == 1)&&(address[12] == 0)){
          for(int i = 0; i < 4; i++){
            mem.data[address.range(14,0)+i][address.range(7,0)][b_address][0] = data.range(7,0);
            rem.data[address.range(14,0)+i][address.range(7,0)][b_address][1] = data.range(15,8);
            wait(burst_delay);
          }
      }
      // Se escriben 8 datos seguidos
      else if((ddress[10] == 1)&&(address[12] == 1)){
        for(int i = 0; i < 8; i++){
          mem.data[address.range(14,0)+i][address.range(7,0)][b_address][0] = data.range(7,0);
          rem.data[address.range(14,0)+i][address.range(7,0)][b_address][1] = data.range(15,8);
          wait(burst_delay);
        }
      }
    }
  }


  void read(int addr, int blck){
    address = addr;
    b_address = blck;
    rd_t.notify(rd_delay, SC_NS);
  }


  int rd(){
    while(true){
      int data_out;
      address = addr;
      b_address = b
      // Se lee solo 1 dato
      if(address[10] == 0){
        data_out.range(7,0) = mem.data[address.range(14,0)][address.range(7,0)][b_address][0];
        data_out.range(15,8) = mem.data[address.range(14,0)][address.range(7,0)][b_address][1];
        return data_out;
      }
      // Se leen 4 datos en rafaga
      else if((ddress[10] == 1)&&(address[12] == 0)){
        for(int i = 0; i < 4; i++){
          data_out.range(7,0) = mem.data[address.range(14,0)+i][address.range(7,0)][b_address][0];
          data_out.range(15,8) = mem.data[address.range(14,0)+i][address.range(7,0)][b_address][1];
          return data_out;
          wait(burst_delay);
        }
      }
      // Se leen 8 datos en rafaga
      else if((ddress[10] == 1)&&(address[12] == 1)){
        for(int i = 0; i < 8; i++){
          data_out.range(7,0) = mem.data[address.range(14,0)+i][address.range(7,0)][b_address][0];
          data_out.range(15,8) = mem.data[address.range(14,0)+i][address.range(7,0)][b_address][1];
          return data_out;
          wait(burst_delay);
        }
      }
    }
  };  // Final del modulo
