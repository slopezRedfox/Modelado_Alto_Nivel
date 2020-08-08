#ifndef TOP_H
#define TOP_H

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "initiator.h"
#include "initiator_2.h"
#include "router.h"
#include "ram.h"
#include "dummy.h"

SC_MODULE(Top)   
{   
    Controler   *controler_1;   
    Controler_2 *controler_2;
    Ram         *ram;
    Router      *router;

    SC_CTOR(Top)
    {
        controler_1 = new Controler("controler_1");
        controler_2 = new Controler_2("controler_2");
        router      = new Router   ("router");

        ram         = new Ram      ("ram");   

        controler_1->socket_initiator.bind(  router->socket_target_C); //CPU
        router     ->socket_initiator_C.bind(controler_1->socket_target);  

        ram    ->socket_initiator.bind(  router->socket_target_A);
        router ->socket_initiator_A.bind(ram->socket_target);

        controler_2->socket_initiator.bind(  router->socket_target_B); //CPU
        router     ->socket_initiator_B.bind(controler_2->socket_target);
    }
};

#endif
