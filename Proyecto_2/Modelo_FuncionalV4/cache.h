#ifndef CACHE_H
#define CACHE_H
// chmod 777
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
#define wr_delay 3
#define rd_delay 2
#define wr_miss_delay 2
#define cln_delay 2
#define pen_delay 5

#define sets_n 4096
#define ways_n 8
#define block_n 4

#define RAM 0xAB00000000

#define DELAY_CACHE 5

//----------------------------------------------------------------------------------------------------------------------
//Clases usadas para memoria cache

//Cada bloque cotiene 4 lineas de 32bits (4 bytes cada una)
class Cache_Block
{
public:
  sc_uint<32> data[block_n];
  sc_uint<18> tag[block_n];

  short order[block_n];
  bool use[block_n];
};

//Cada set contiene 8 bloques, ya que es un 8 ways
//Como es asociativa
class Cache_Set
{
public:
  Cache_Block block[ways_n];
};

//En la memoria cache hay 4096 Sets
class mem_cache
{
public:
  Cache_Set sets[sets_n];
};

//----------------------------------------------------------------------------------------------------------------------
//Modulo de cache

struct Cache : sc_module
{
    tlm_utils::simple_initiator_socket<Cache> socket_initiator;
    tlm_utils::simple_target_socket<Cache> socket_target;
    const sc_time LATENCY;

    //Se declara el constructor y se establece la latencia a 10ns
    SC_CTOR(Cache) : 
        socket_target("socket_target"), 
        socket_initiator("socket_initiator"),
        LATENCY(DELAY_CACHE, SC_NS)
    {
        // Register callbacks for incoming interface method calls
        socket_target.register_nb_transport_fw(this, &Cache::nb_transport_fw);
        socket_initiator.register_nb_transport_bw(this, &Cache::nb_transport_bw);

        //Se establece una funcion recurente
        SC_THREAD(thread_process_to_bw);
        SC_THREAD(thread_process_to_fw);
        SC_THREAD(wr);
        SC_THREAD(rd);
    }

    //==============================================================================================
    //                                   FUNCIONES DE TARGET
    //==============================================================================================

    //----------------------------------------------------------------------------------------------
    // Thread Target
    void thread_process_to_fw()
    {
        while (true)
        {
            wait(do_target_t.default_event()); 

            trans_pending = trans_pending_queue.front();
            phase_pending = phase_pending_queue.front();
            delay_pending = delay_pending_queue.front();

            trans_pending_queue.pop();
            phase_pending_queue.pop();
            delay_pending_queue.pop();

            tlm::tlm_phase phase;
            ID_extension *id_extension = new ID_extension;
            trans_pending->get_extension(id_extension);
            *id_extension_initiator = *id_extension;

            //Parametros del paquete de transaccion
            tlm::tlm_command cmd = trans_pending->get_command();
            sc_dt::uint64    adr = trans_pending->get_address();
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
            trans_pending->set_response_status(tlm::TLM_OK_RESPONSE);
            delay_pending = sc_time(DELAY_CACHE, SC_NS);

            //********************************************************************************
            // SE MODIFICA SEGUN LO QUE SE NECESITE CUANDO SE VA A HACER LECTURAS/ESCRITURAS
            //********************************************************************************
            if (cmd == tlm::TLM_READ_COMMAND)
            {
                data_Target = ptr;
                address_Target = static_cast<sc_uint<32>>(adr & 0xFFFFFFFF);
                read();
                wait(target_done_t);

                memcpy(&data_aux_Target, ptr, 4);
                cout << "Estimador " << endl;
                cout << "Estimador cache read data    : " << (float)(data_aux_Target)/pow(2,21) << " TRANS ID " << id_extension->transaction_id << endl;
                cout << "Estimador cache read data hex: " << data_aux_Target                    << " TRANS ID " << id_extension->transaction_id << endl;
                cout << "Estimador cache read addr    : " << address_Target                     << " TRANS ID " << id_extension->transaction_id << endl;
            }

            else if (cmd == tlm::TLM_WRITE_COMMAND)
            {
                data_Target = ptr;
                address_Target = static_cast<sc_uint<32>>(adr & 0xFFFFFFFF);
                
                memcpy(&data_aux_Target, ptr, 4);
                cout << "Estimador " << endl;
                cout << "Estimador cache write data    : " << (float)(data_aux_Target)/pow(2,21) << " TRANS ID " << id_extension->transaction_id << endl;
                cout << "Estimador cache write data hex: " << data_aux_Target                    << " TRANS ID " << id_extension->transaction_id << endl;
                cout << "Estimador cache write addr    : " << address_Target                     << " TRANS ID " << id_extension->transaction_id << endl;
                
                write();
                wait(target_done_t);
            }
            //********************************************************************************
            //********************************************************************************

            tlm::tlm_sync_enum status;
            phase = tlm::BEGIN_RESP;

            wait(delay_pending);
            cout << "1 - " << name() << "    BEGIN_RESP SENT    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;

            status = socket_target->nb_transport_bw(*trans_pending, phase, delay_pending);
            wait(delay_pending);

            switch (status)
                case tlm::TLM_ACCEPTED:

                    cout << "1 - " << name() << "    END_RESP   SENT    " << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
                    phase = tlm::END_RESP;

                    socket_target->nb_transport_bw(*trans_pending, phase, delay_pending); // Non-blocking transport call

            /*/********************************************************************************
            //                     SOLO SE USA PARA PRUEBAS DE LA CACHE
            //********************************************************************************
            cout << endl;
            cout << "Estimador index: " << index << "| offset: " << offset << endl;
            cout << endl;
            cout << "Estimador *******************************************************" << endl;
            cout << "Estimador Memoria Cache" << endl;
            for (int j = 0; j < ways_n; j++)
            {
                cout << "Estimador Way #" << dec << j << " Tag: " << hex << mem.sets[index].block[j].tag[offset] << " | data: " << hex << mem.sets[index].block[j].data[offset] << endl;
            }
            cout << endl;
            cout << "Estimador *******************************************************" << endl;

            //********************************************************************************
            //**********************************************************************************/
        }
    }

    //----------------------------------------------------------------------------------------------

    virtual tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload &trans,
                                                tlm::tlm_phase &phase,
                                                sc_time &delay)
    {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64    adr = trans.get_address();
        unsigned char*   ptr = trans.get_data_ptr();   
        unsigned int     len = trans.get_data_length();   
        unsigned char*   byt = trans.get_byte_enable_ptr();   
        unsigned int     wid = trans.get_streaming_width();   

        //Se hace una copia del ID de la transaccion
        ID_extension* id_extension = new ID_extension;
        ID_extension* id_extension2 = new ID_extension;
        
        trans.get_extension(id_extension);
        *id_extension2 = *id_extension;

        //
        // PRIMERA FASE DE LA TRANSACCION
        //
        if (phase == tlm::BEGIN_REQ)
        {
            // Verifica errores
            if (byt != 0)
            {
                cout << "ERROR 1" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;   
                trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
                return tlm::TLM_COMPLETED;
            }

            if (len > 4 || wid != 0)
            {
                cout << "ERROR 2" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;   
                trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
                return tlm::TLM_COMPLETED;
            }

            //==========================================================================
            // Se hace una copia de las transacciones para dar lugar a otras
            tlm::tlm_generic_payload* trans_aux = new tlm::tlm_generic_payload;
            int * data_ptr = new int;
            
            memcpy(data_ptr, ptr, 4);

            trans_aux->set_command( cmd );   
            trans_aux->set_address( adr );   
            
            if ( cmd == tlm::TLM_WRITE_COMMAND ){
                trans_aux->set_data_ptr( reinterpret_cast<unsigned char*>(data_ptr) );
            }

            else if ( cmd == tlm::TLM_READ_COMMAND ){
                trans_aux->set_data_ptr( ptr );
            }
            
            trans_aux->set_data_length( len );   
            trans_aux->set_byte_enable_ptr( byt );
            trans_aux->set_extension( id_extension2 );
            
            //==========================================================================
            // Se hace un push en los queue para dar espacio a otras transacciones
            trans_pending_queue.push(trans_aux);
            phase_pending_queue.push(phase);
            delay_pending_queue.push(delay);

            cout << "1 - " << name() << "    BEGIN_REQ  RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
            do_target_t.notify(0, SC_NS);
            return tlm::TLM_ACCEPTED;
        }

        //
        // SEGUNDA FASE DE LA TRANSACCION
        //
        else if (phase == tlm::END_REQ)
        {
            cout << "1 - " << name() << "    END_REQ    RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
            return tlm::TLM_COMPLETED;
        }

        else
        {
            return tlm::TLM_COMPLETED;
        }
    };

    //==============================================================================================
    //                                   FUNCIONES DE INICIADOR
    //==============================================================================================

    //----------------------------------------------------------------------------------------------
    // Thread Iniciador
    void thread_process_to_bw(){
        
        tlm::tlm_generic_payload trans;
        sc_time delay = sc_time(DELAY_CACHE, SC_NS);

        while(true)
        {
            wait(do_initiator_t);                

            trans.set_extension( id_extension_initiator );

            tlm::tlm_phase phase = tlm::BEGIN_REQ;
            tlm::tlm_command cmd = static_cast<tlm::tlm_command>(comando_Initiator); //comando_Initiator = (0 read, 1 write)
            
            //Parametros de trans
            trans.set_command( cmd );   
            trans.set_address( address_Initiator );   
            trans.set_data_ptr( reinterpret_cast<unsigned char*>(&data_Initiator) ); //Este es un puntero
            trans.set_data_length( 4 );   
            trans.set_byte_enable_ptr( 0 );

            tlm::tlm_sync_enum status;

            wait( delay );
            cout  << "0 - "<< name() << " BEGIN_REQ  SENT    " << " TRANS ID " << id_extension_initiator->transaction_id << " at time " << sc_time_stamp() << endl;

            status = socket_initiator->nb_transport_fw(trans, phase, delay );  // Non-blocking transport call
            wait( delay );

            switch (status)
            {
                case tlm::TLM_ACCEPTED:   
                
                    cout  << "0 - "<< name() << " END_REQ    SENT    " << " TRANS ID " << id_extension_initiator->transaction_id << " at time " << sc_time_stamp() << endl;
                    phase = tlm::END_REQ; 
                    
                    status = socket_initiator->nb_transport_fw( trans, phase, delay );  // Non-blocking transport call
                    break;   
            
                case tlm::TLM_UPDATED:

                    //None to do

                case tlm::TLM_COMPLETED:   
            
                    if (trans.is_response_error() )   
                        SC_REPORT_ERROR("TLM2", "Response error from nb_transport_fw");   

                    cout << endl;
                    cout  << "0 - "<< "trans/fw = { " << (cmd ? 'W' : 'R') << ", " << hex << 0 << " } , data = "   
                            << hex << data_Initiator << " at time " << sc_time_stamp() << ", delay = " << delay << endl;
                    cout << endl;
                    break;   
            }
            
            wait(0, SC_NS);   
            initiator_done_t.notify();
        }
    }
    
    //----------------------------------------------------------------------------------------------
    
    virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans,
                                                tlm::tlm_phase& phase,
                                                sc_time& delay ){
        
        tlm::tlm_command cmd    =  trans.get_command();   
        sc_dt::uint64    adr    =  trans.get_address();   
        int              data_p;
        memcpy(&data_p, trans.get_data_ptr(), 4);

        ID_extension* id_extension = new ID_extension;
        trans.get_extension( id_extension ); 

        if (phase == tlm::BEGIN_RESP) 
        {                        
            // Initiator obliged to check response status   
            if (trans.is_response_error() )   
                SC_REPORT_ERROR("TLM2", "Response error from nb_transport");   

            cout  << "0 - "<< name () << " BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension_initiator->transaction_id << " at time " << sc_time_stamp() << endl;
            return tlm::TLM_ACCEPTED;
        } 

        else if (phase == tlm::END_RESP) 
        {
            cout  << "0 - "<< name() << " END_RESP   RECEIVED" << " TRANS ID " << id_extension_initiator->transaction_id << " at time " << sc_time_stamp() << endl;
            initiator_done_Resp_t.notify();
            return tlm::TLM_COMPLETED;
        }

        else
        {
            initiator_done_Resp_t.notify();
            return tlm::TLM_COMPLETED;
        }
    }


    //==============================================================================================
    //==============================================================================================

    // ****************
    // INTERNALS
    // ****************

    void write()
    {
        wr_t.notify(wr_delay, SC_NS);
    }

    void read()
    {
        rd_t.notify(rd_delay, SC_NS);
    }

    void reorder(int i, int tag, int index, int offset, int data)
    {
        for (int w = i; w > 0; w--)
        {
            mem.sets[index].block[w].tag[offset] = mem.sets[index].block[w - 1].tag[offset];
            mem.sets[index].block[w].data[offset] = mem.sets[index].block[w - 1].data[offset];
            mem.sets[index].block[w].use[offset] = mem.sets[index].block[w - 1].use[offset];
            mem.sets[index].block[w].order[offset] = w;
        }

        mem.sets[index].block[0].tag[offset] = tag;
        mem.sets[index].block[0].data[offset] = data;
        mem.sets[index].block[0].use[offset] = 1;
        mem.sets[index].block[0].order[offset] = 0;
    }

    //Escritura
    void wr()
    {
        while (true)
        {
            //Espera tiempo de escritura
            wait(wr_t);

            tag = address_Target.range(31, 14);
            index = address_Target.range(13, 2);
            offset = address_Target.range(1, 0);
            memcpy(&data_aux_Target, data_Target, 4);
            
            if (tag == 0x10f00)
            {
                mem.sets[index].block[0].data[offset]  = data_aux_Target;
                data_Initiator = data_aux_Target;  
                address_Initiator = address_Target | RAM;
                comando_Initiator = 1;
                do_initiator_t.notify();
                wait(initiator_done_Resp_t);
            }

            else
            {
                for (int i = 0; i < ways_n; i++)
                {
                    //Este condisional se traduce como, verifique si el espacio
                    //en memoria de offset en el set numero index esta disponible
                    if (mem.sets[index].block[i].use[offset] == 0)
                    {
                        wait(wr_miss_delay, SC_NS);
                        reorder(i, tag, index, offset, data_aux_Target);
                        mem.sets[index].block[0].data[offset]  = data_aux_Target;
                        data_Initiator = data_aux_Target;  
                        address_Initiator = address_Target | RAM;
                        comando_Initiator = 1;
                        do_initiator_t.notify();
                        wait(initiator_done_Resp_t);
                        break;
                    }
                    else
                    {
                        if (i == ways_n - 1)
                        {
                            wait(wr_miss_delay, SC_NS);
                            reorder(i, tag, index, offset, data_aux_Target);
                            mem.sets[index].block[0].data[offset]  = data_aux_Target;
                            data_Initiator = data_aux_Target;  
                            address_Initiator = address_Target | RAM;
                            comando_Initiator = 1;
                            do_initiator_t.notify();
                            wait(initiator_done_Resp_t);
                        }
                    }
                }
            }
            target_done_t.notify();
        }
    }

    //Lectura
    void rd()
    {
        while (true)
        {
            //Espera tiempo de lectura
            wait(rd_t);

            flag = 0;
            tag = address_Target.range(31, 14);
            index = address_Target.range(13, 2);
            offset = address_Target.range(1, 0);

            if (tag == 0x10f00)
            {
                memcpy(&data_aux_Target, data_Target, 4);

                data_Initiator = data_aux_Target;
                address_Initiator = address_Target | RAM;
                comando_Initiator = 0;
                do_initiator_t.notify();
                wait(initiator_done_Resp_t);

                mem.sets[index].block[0].data[offset]  = data_Initiator;

                //Se copia el dato en 'data' y sale del bucle
                memcpy(data_Target, &data_Initiator, 4);            
            }

            else
            {
                for (int i = 0; i < ways_n; i++)
                {
                    if (mem.sets[index].block[i].use[offset] == 1)
                    {
                        //Si se encuentra
                        if (mem.sets[index].block[i].tag[offset] == tag)
                        {
                            data_aux_Target = mem.sets[index].block[i].data[offset];

                            //Se copia el dato en 'data' y sale del bucle
                            memcpy(data_Target, &data_aux_Target, 4);
                            reorder(i, tag, index, offset, data_aux_Target);
                            break;
                        }

                        //Si se llega al final de la cache y no se encuentra
                        //la diredccion se lebanda la bandera para buscar el
                        //dato fuera de la cache
                        else if (i == ways_n - 1)
                        {
                            memcpy(&data_aux_Target, data_Target, 4);

                            data_Initiator = data_aux_Target;
                            address_Initiator = address_Target | RAM;
                            comando_Initiator = 0;
                            do_initiator_t.notify();
                            wait(initiator_done_Resp_t);

                            reorder(i, tag, index, offset, data_Initiator);
                            memcpy(data_Target, &data_Initiator, 4);

                            flag = 1;
                            wait(pen_delay, SC_NS);
                        }
                    }

                    else
                    {
                        memcpy(&data_aux_Target, data_Target, 4);

                        data_Initiator = data_aux_Target;
                        address_Initiator = address_Target | RAM;
                        comando_Initiator = 0;
                        do_initiator_t.notify();
                        wait(initiator_done_Resp_t);

                        reorder(i, tag, index, offset, data_Initiator);
                        memcpy(data_Target, &data_Initiator, 4);

                        flag = 1;
                        wait(pen_delay, SC_NS);
                        break;
                    }
                }
            }
            target_done_t.notify();
        }
    }

    //Variables de puerto Target
    sc_event_queue  do_target_t; 
    std::queue<tlm::tlm_generic_payload*> trans_pending_queue;   
    std::queue<tlm::tlm_phase>            phase_pending_queue;   
    std::queue<sc_time>                   delay_pending_queue;

    tlm::tlm_generic_payload* trans_pending;   
    tlm::tlm_phase phase_pending;   
    sc_time delay_pending;
    
    sc_event target_done_t;
    int data_aux_Target;
    unsigned char *data_Target;
    sc_uint<32> address_Target;

    //Variables de puerto Iniciador  
    sc_event do_initiator_t;
    sc_event initiator_done_t, initiator_done_Resp_t;
    int data_Initiator;  
    long int address_Initiator;
    bool comando_Initiator;
    ID_extension* id_extension_initiator = new ID_extension;

    //Variables internas
    mem_cache mem;
    sc_event wr_t, rd_t;
    int tag;
    int index;
    int offset;
    int Aux;
    bool flag;
};

#endif