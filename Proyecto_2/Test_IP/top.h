#ifndef TOP_H
#define TOP_H

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "estimador.h"
#include "ram.h"

SC_MODULE(Top)   
{   
    Estimador *estimador;
    Ram     *ram;

    SC_CTOR(Top)
    {
        estimador = new Estimador("estimador");
        ram   = new Ram    ("ram");   

        estimador->socket_initiator.bind(ram  ->socket_target);
        ram      ->socket_initiator.bind(estimador->socket_target);
    }
};

#endif
