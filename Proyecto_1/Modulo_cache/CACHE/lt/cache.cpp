//-----------------------------------------------------
#include "systemc.h"

/*
Modulo 
Tiempo de acceso de 8 ciclos
Cache L2 de 512KB 8-way set-associative

Frecuencia minima CPU = 0.8GHz 
Frecuencia maxima CPU = 2GHz


        Input                   Output
                 ____________   
                 |          |-- on (1bit)
        data   --|          |   
      (32bits)   | Cache L2 |-- rst (1bit)
                 |  512kBs  |   
        adress --|          |-- r_w 
      (32bits)   |          |   Read/Write (1bit)
                 |          |   
          flag --|          | 
Pos/Neg (1bit)   |          |   
                 |          |
           clk --|          |
                 |__________|   

Data   = Es un in/out donde se leen/escriben los datos de tamaño de 8 bits
Adress = Es la dirección del disco duro donde están los datos deseados (bus de 8 bits)
on     = Apaga/Enciende el modulo.
Reset  = Reinicia/Limpia la memoria.
r_w    = Indica si se desea Leer o Escribir datos en la Cache (1 = Leer / 0 = Escribir)
Flag   = Indica si el dato que se desea leer se encuentra o no en la Cache.
         (0 = Dato se encuentara en cache / 1 = Dato NO se encuentara en cache)
*/

class mem_cache {
  public:          
    int data; 
    int address;
};

SC_MODULE (cache) {
  //--------------------------------------------------------
  //-------------------- VARIABLES -------------------------
  //--------------------------------------------------------
  int           data; 
  int        address;
  
  sc_in<bool>     on;
  sc_in<bool>    rst;
  sc_in<bool>    r_w;
  sc_out<bool>  flag;

  //Reloj del sistema
  sc_in<bool>    clk;

  //Memoria de la cache
  int ext = 3;
  mem_cache mem[3];

  //Puntero de escritura en memoria
  int cnt = 0;

  //Bandera de evento (systemC)
  sc_event del_t;

  //--------------------------------------------------------
  //------------------- CONSTRUCTOR ------------------------
  //--------------------------------------------------------
  SC_HAS_PROCESS(cache);
    cache(sc_module_name cache) {
    SC_THREAD(wr);
  }

  //--------------------------------------------------------
  //-------------------- FUNCIONES  ------------------------
  //--------------------------------------------------------
  //Delay write
  void write(int addr, int dat) {
    data = dat;
    address = addr;
    del_t.notify(4, SC_NS);
  }

  //Escritura
  void wr() {
    while(true) {
      //Espera tiempo de escritura
      wait(del_t);

      //Escribe el dato y la direccion en el espacio del cache
      mem[cnt].data    = data;
      mem[cnt].address = address;

      //Se le suma uno al puntero de escritura en cache
      //pero si se llego al final de la cache, se regresa al inicio
      if (cnt == ext-1){
        cnt = 0;
      }
      else{
        cnt = cnt +1;
      }
    }
  }

  //Lectura
  int read(int addr) {
    //Se busca la direccion en la cache
    for (int i = 0; i < ext; i++){
      //Si se encuentra
      if (mem[i].address == addr){
        //Se copia el dato en 'data' y sale del bucle
        data = mem[i].data;
        flag = 0 ;
        return data;
      }

      //Si se llega al final de la cache y no se encuentra 
      //la diredccion se lebanda la bandera para buscar el
      //dato fuera de la cache
      else if(i==ext-1){
        flag = 1;
        return 0x0;
      }
    }   
  }
};