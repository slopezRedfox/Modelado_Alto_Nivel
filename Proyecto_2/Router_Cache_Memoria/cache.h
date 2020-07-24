#ifndef TARGET_H
#define TARGET_H

/*
Modulo 
Tiempo de acceso de 8 ciclos
Cache L2 de 512KB 8-way set-associative
Tiempo de acceso tipico es de 5-25ns

Frecuencia = 600 MHz

        Input                   Output
                 ____________   
                 |          |-- flag
        r_w    --|          |   Pos/Neg (1bit)
      (1 bits)   | Cache L2 |
                 |   512KB  |   
        adress --|          |-- data 
      ( 32bits)  |          |   (32 bit)
                 |          |   
          on   --|          | 
       ( 1bit)   |          |   
                 |          |
                 |          |
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

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"


//Constantes de memoria cache
#define wr_delay 5
#define rd_delay 2
#define wr_miss_delay 2
#define cln_delay 5
#define pen_delay 100

#define sets_n 4
#define ways_n 8
#define block_n 4096

//----------------------------------------------------------------------------------------------------------------------
//Clases usadas para memoria cache

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



//----------------------------------------------------------------------------------------------------------------------
//Modulo de cache

struct Cache: sc_module {   
  
  tlm_utils::simple_target_socket<Cache> socket;
  const sc_time LATENCY;   
  
  //Se declara el constructor y se establece la latencia a 10ns
  SC_CTOR(Cache) : socket("socket"), LATENCY(10, SC_NS){

    // Register callbacks for incoming interface method calls
    socket.register_nb_transport_fw(this, &Cache::nb_transport_fw);

    //Se establece una funcion recurente
    SC_THREAD(thread_process);   
    SC_THREAD(wr);
    SC_THREAD(rd);
  }

  //==============================================================================================
  void thread_process(){
    
    while (true) {
    
      // wait
      wait(e1); 
   
      tlm::tlm_phase phase;
      ID_extension* id_extension = new ID_extension;
      trans_pending->get_extension( id_extension ); 
      
      //Entre las diferencias con la funcion anterior es que aqui se pide el tipo de comando
      //Si se esta hablando de una escritura o una lectura. Otra diferencia es que en este
      //apartado si se hara uso de esta informacion que se extrae de la transaccion
      tlm::tlm_command cmd = trans_pending->get_command();
      sc_dt::uint64    adr = trans_pending->get_address();

      //La otra diferencia es que aqui si se extrae los datos de la transferencia
      unsigned char*   ptr = trans_pending->get_data_ptr();   
      unsigned int     len = trans_pending->get_data_length();   
      unsigned char*   byt = trans_pending->get_byte_enable_ptr();   
      unsigned int     wid = trans_pending->get_streaming_width();   

      //Al igual que en el punto anterior se revisa que todos los datos de la transaccione esten bien
      if (byt != 0 || wid != 0 || len > 4)   
        SC_REPORT_ERROR("TLM2", "Target does not support given generic payload transaction");   
      
      //
      //                            IMPORTANTE
      //
      // Se pasan los parametros a las variables globales, dependiendo si se esta
      // en escritura o lectura

      // Obliged to set response status to indicate successful completion   
      trans_pending->set_response_status( tlm::TLM_OK_RESPONSE );  
      delay_pending= sc_time(10, SC_NS);

      //********************************************************************************
      // SE MODIFICA SEGUN LO QUE SE NECESITE CUANDO SE VA A HACER LECTURAS/ESCRITURAS
      //********************************************************************************
      if ( cmd == tlm::TLM_READ_COMMAND ){
        data = ptr;
        address = static_cast<sc_uint<32> >( adr & 0xFFFFFFFF);
        read();
        wait(done_t);

        if (flag == 1){
          trans_pending->set_address( 0xF00000000);
        }
      }

      else if ( cmd == tlm::TLM_WRITE_COMMAND ){
        data = ptr;
        address = static_cast<sc_uint<32> >( adr & 0xFFFFFFFF);
        write();
        wait(done_t);
      }
      //********************************************************************************
      //********************************************************************************
      
      tlm::tlm_sync_enum status;
      phase  = tlm::BEGIN_RESP; 
      
      wait( sc_time(10, SC_NS) );
      cout  << "1 - "   << name() << "    BEGIN_RESP SENT    " << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
      
      status = socket->nb_transport_bw( *trans_pending, phase, delay_pending );   

      switch (status)     
        case tlm::TLM_ACCEPTED:   
          
          wait( sc_time(10, SC_NS) );          
          cout  << "1 - "   << name() << "    END_RESP   SENT    " << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
          phase = tlm::END_RESP;
          
          socket->nb_transport_bw( *trans_pending, phase, delay_pending );  // Non-blocking transport call
      

      //********************************************************************************
      //                     SOLO SE USA PARA PRUEBAS DE LA CACHE
      //********************************************************************************
      cout << endl;
      cout << "index: " << index << "| offset: " << offset << endl;
      cout << endl;
      for (int j = 0; j < ways_n; j++){
        cout << "Way #" << dec << j << " Tag: " << hex << mem.sets[index].block[j].tag[offset] << " | data: " << hex << mem.sets[index].block[j].data[offset] << endl;
      }
      cout << endl;
      //********************************************************************************
      //********************************************************************************

    }   
  }   
  
  //==============================================================================================

  // TLM2 non-blocking transport method
  //
  // Se igresan tres variables:
  //   trans: Transaccion entrante
  //   phase: Indica la phase en la que se encuentra la transaccion
  //   delay: Tiempo que tarda la transaccion

  virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, 
                                              sc_time& delay){
    
    //Estos son los la lista de atributos usados en el non-bloking.
    //Se busca la direccion de la transferencia (no se usa)
    sc_dt::uint64    adr = trans.get_address();

    //Se busca el dato del tamaño del bloque de datos que se esta transfiriendo (no se usa)
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr(); //Se busca el bit enable de datos
    unsigned int     wid = trans.get_streaming_width(); //PREGUNTA: ???? (no se usa)

    //Se le coloca un ID a cada transaccion
    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension ); //Se extrae el ID de la transaccion

    //
    // PRIMERA FASE DE LA TRANSACCION
    //
    if(phase == tlm::BEGIN_REQ){
      // Primero se pregunta si el dato a transmitir esta abilitado
      if (byt != 0) {
        //
        trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
        return tlm::TLM_COMPLETED;
      }
      
      // Luego se pregunta si el tamaño del paquete es el adecuado
      if (len > 4 || wid != 0) {
        trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
        return tlm::TLM_COMPLETED;
      }

      // Se pasa los datos a las variables globales de la estructura
      // Que estan declaradas al final de la misma, para poder ser
      // trabajados en la funcion thread
      trans_pending = &trans;
      phase_pending = phase;
      delay_pending = delay;

      //Se imprime que se inicio el request
      //Se indica el ID de la transaccion y el tiempo en el que inicia
      wait(delay);
      cout  << "1 - "   << name() << "    BEGIN_REQ  RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;            

      // Se hace un notify de e1 para permitir a la funcion thread ejecutar
      e1.notify();

      //Se envia que la transaccion ha sido aceptada
      return tlm::TLM_ACCEPTED;
    }


    //
    // SEGUNDA FASE DE LA TRANSACCION
    //
    if(phase == tlm::END_REQ){
      //Se espera el delay ingresado en la funcion
      wait(delay);
      //Se imprime en pantalla el id de la transaccion y el tiempo de ejecucion en el que se encuentra el proceso
      cout  << "1 - "   << name() << "    END_REQ    RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      
      //Se imprime que se termino el request
      //Devuelve la confirmacion de que se TERMONO la transaccion
      return tlm::TLM_COMPLETED;
    }
  }
  
  //==============================================================================================

  void write() {
    wr_t.notify(wr_delay, SC_NS);
  }

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

      tag    = address.range(31,14);
      index  = address.range(13,2);
      offset = address.range(1,0);

      memcpy(&data_aux, data, 4);
      //mem.sets[index].block[0].data[offset]  = data_aux;

      for (int i = 0; i < ways_n; i++){
        //Este condisional se traduce como, verifique si el espacio
        //en memoria de offset en el set numero index esta disponible
        if (mem.sets[index].block[i].use[offset] == 0){
          wait(i*wr_miss_delay,SC_NS);
          reorder(i, tag, index, offset, data_aux);
          break;
        }
        else{
          if (i == ways_n - 1){
            wait(i*wr_miss_delay + cln_delay,SC_NS);
            reorder(i, tag, index, offset, data_aux);
          }
        }
      }

      done_t.notify();
    }
  }

  //Lectura
  void rd() {
    while(true) {

      //Espera tiempo de lectura
      wait(rd_t);

      flag = 0;
      tag    = address.range(31,14);
      index  = address.range(13,2);
      offset = address.range(1,0);


      for (int i = 0; i < ways_n; i++){
        if (mem.sets[index].block[i].use[offset] == 1){
          
          //Si se encuentra
          if (mem.sets[index].block[i].tag[offset] == tag){
            
            data_aux = mem.sets[index].block[i].data[offset]; 

            //Se copia el dato en 'data' y sale del bucle
            memcpy(data, &data_aux, 4);
            reorder(i, tag, index, offset, data_aux);
            break;
          }

          //Si se llega al final de la cache y no se encuentra 
          //la diredccion se lebanda la bandera para buscar el
          //dato fuera de la cache
          else if(i == ways_n - 1){
            flag = 1;
            data_aux = mem.sets[index].block[6].data[offset];
            memcpy(data, &data_aux, 4);
            wait(pen_delay,SC_NS);
          }
        }

        else {
          flag = 1;
          data_aux = 0;
          memcpy(data, &data_aux, 4);
          wait(pen_delay,SC_NS);
          break;
        }
      }

      done_t.notify();
    }
  }
  
  //Parametros nuevos
  sc_event  e1; 
  tlm::tlm_generic_payload* trans_pending;   
  tlm::tlm_phase phase_pending;   
  sc_time delay_pending;
  sc_event done_t;
  int data_aux;
  
  //
  mem_cache mem;
  sc_event wr_t, rd_t;
  sc_uint<32> address;
  int tag;
  int index;
  int offset;
  int Aux;

  //Modificados
  unsigned char* data;
  bool flag;
};

#endif