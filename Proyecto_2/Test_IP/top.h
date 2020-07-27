#ifndef TOP_H
#define TOP_H

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "initiator.h"
//#include "dummy.h"
#include "ram.h"

SC_MODULE(Top)   
{   
    Controler *controler;
    Ram     *ram;

    SC_CTOR(Top)
    {
        controler = new Controler("controler");
        ram   = new Ram    ("ram");   

        controler->socket_initiator.bind(ram  ->socket_target);
        ram      ->socket_initiator.bind(controler->socket_target);
    }
};

#endif
