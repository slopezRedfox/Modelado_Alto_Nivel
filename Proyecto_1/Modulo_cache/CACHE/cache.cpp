//-----------------------------------------------------
#include "systemc.h"

/*
Modulo 
Tiempo de acceso de 8 ciclos
Cache L2 de 512KB 8-way set-associative
Tiempo de acceso tipico es de 5-25ns

Frecuencia = 600 MHz

        Input                   Output
                 ____________   
                 |          |-- on (1bit)
        data   --|          |   
      (32bits)   | Cache L2 |-- rst (1bit)
                 |   512KB  |   
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

¿Eventos del modulo?
Se deben considerar los tiempos de ejecucion de:

- Tiempo de lectura de datos
- Tiempo de escritura
- Tiempo de miss (100-300 ciclos)
- Tiempo de hit  (5-12 ciclos)
- Tiempo de borrado de datos
- Tiempo de escritura con memoria llena
- Tiempo de lectura con memoria llena
- Timpo de refrescamiento

------------------------------------------------------------------------------------------
Se divide en 2^12 sets de memoria de 8 bloques de memoria cada uno y en cada bloque hay 
4 lineas de 32bits (16 bytes). Por lo que la maya de cache queda como:

OFFSET = 2  bits
INDEX  = 12 bits
TAG    = 18 bits

8 bloques * 4 lineas de 32 bits = 128 bytes
128 bytes * 2^12 Sets           = 524288 bytes = 512KB


Entonces, el direccionamiento en cache quedaria de la siguiete manera:
 ______________________________________
|                 |           |        |
|                 |           |        |
|       TAG       |   INDEX   | OFFSET |
|    (18 Bits)    | (12 Bits) |(2 Bits)|
|                 |           |        |
|_________________|___________|________|

*/

//IMPORTANTE
//Declaracion de la clase de memoria cache

//Constantes de memoria
#define wr_delay 5
#define rd_delay 2
#define wr_miss_delay 2
#define cln_delay 5
#define pen_delay 100

#define sets_n 4096
#define ways_n 4
#define block_n 4

//Cada bloque cotiene 4 lineas de 32bits (4 bytes cada una)
class Cache_Block {
  public:
    sc_uint <32> data[block_n];
    sc_uint <18> tag[block_n];

    short order[block_n];
    bool use[block_n];
};

//Cada set contiene 8 bloques, ya que es un 8 ways
//Como es asociativa
class Cache_Set {
  public:
    Cache_Block  block[ways_n];
};

//En la memoria cache hay 4096 Sets
class mem_cache {
  public:     
    Cache_Set sets[sets_n];
};


SC_MODULE (cache) {
  //--------------------------------------------------------
  //-------------------- VARIABLES -------------------------
  //--------------------------------------------------------
  sc_inout<sc_uint<32>>   data; 
  sc_in<sc_uint<32>>   address;
  
  sc_in<bool>               on;
  sc_in<bool>              rst;
  sc_in<bool>              r_w;
  sc_out<bool>            flag;

  //Reloj del sistema
  sc_in<bool>              clk;

  //Creasion de la memoria cache
  mem_cache mem;
  sc_uint<32> Addr;
  int tag;
  int index;
  int offset;

  //Auxiliar
  sc_uint<2> Aux;
  int pila[8];
  //Banderas de evento (systemC)
  sc_event rd_t, wr_t, wait_t;

  //--------------------------------------------------------
  //------------------- CONSTRUCTOR ------------------------
  //--------------------------------------------------------

  //Unifica las señales de on y r_w
  //De este modo es mas facil trabajar posteriormente
  void do_cache(){
    Aux[0] = on;
    Aux[1] = r_w;
    
    //Solo para debugge
    //cout << "Flag: " << Aux << endl;
  }

  SC_HAS_PROCESS(cache);
    cache(sc_module_name cache) {
    SC_THREAD(wr);
    SC_THREAD(rd);

    SC_METHOD(do_cache);
    sensitive << on << r_w;
  }

  //--------------------------------------------------------
  //-------------------- FUNCIONES  ------------------------
  //--------------------------------------------------------

  //Delay de Escritura
  void write() {
    wr_t.notify(wr_delay, SC_NS);
  }
  
  //Delay de Lectura
  void read() {
    rd_t.notify(rd_delay, SC_NS);  
  }

  void reorder(int i, int tag, int index, int offset, int data){
    for (int w = i; w > 0; w--){
      mem.sets[index].block[w].tag[offset]   = mem.sets[index].block[w - 1].tag[offset];
      mem.sets[index].block[w].data[offset]  = mem.sets[index].block[w - 1].data[offset];
      mem.sets[index].block[w].use[offset]   = mem.sets[index].block[w - 1].use[offset];
      mem.sets[index].block[w].order[offset] = w;
    }

    mem.sets[index].block[0].tag[offset]   = tag;
    mem.sets[index].block[0].data[offset]  = data;
    mem.sets[index].block[0].use[offset]   = 1;
    mem.sets[index].block[0].order[offset] = 0;
  }

  //Escritura
  void wr() {
    while(true) {
      //Espera tiempo de escritura
      wait(wr_t);
      
      if(Aux == 1){
        Addr = address.read();

        tag    = Addr.range(31,14);
        index  = Addr.range(13,2);
        offset = Addr.range(1,0);

        for (int i = 0; i < ways_n; i++){
          //Este condisional se traduce como, verifique si el espacio
          //en memoria de offset en el set numero index esta disponible
          if (mem.sets[index].block[i].use[offset] == 0){
            wait(i*wr_miss_delay,SC_NS);
            reorder(i, tag, index, offset, data.read());
            break;
          }
          else{
            if (i == ways_n - 1){
              wait(i*wr_miss_delay + cln_delay,SC_NS);
              reorder(i, tag, index, offset, data.read());
            }
          }
        }
      }
    }
  }

  //Lectura
  void rd() {
    while(true) {

      //Espera tiempo de lectura
      wait(rd_t);
      
      //Si se esta en modo de lectura se sigue adelante
      if(Aux == 3){
        flag = 0;
        Addr = address.read();

        tag    = Addr.range(31,14);
        index  = Addr.range(13,2);
        offset = Addr.range(1,0);

        for (int i = 0; i < ways_n; i++){
          if (mem.sets[index].block[i].use[offset] == 1){
            
            //Si se encuentra
            if (mem.sets[index].block[i].tag[offset] == tag){

              //Se copia el dato en 'data' y sale del bucle
              data = mem.sets[index].block[i].data[offset];              
              int data_aux = mem.sets[index].block[i].data[offset];
              reorder(i, tag, index, offset, data_aux);
              break;
            }

            //Si se llega al final de la cache y no se encuentra 
            //la diredccion se lebanda la bandera para buscar el
            //dato fuera de la cache
            else if(i == ways_n - 1){
              flag = 1;
              data = 0;
              wait(pen_delay,SC_NS);
            }
          }
        }
      }
    }  
  }
};