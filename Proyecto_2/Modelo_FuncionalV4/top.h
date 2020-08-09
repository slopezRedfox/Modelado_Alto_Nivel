#ifndef TOP_H
#define TOP_H

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "cpu.h"
#include "router_1.h"
#include "router_2.h"
#include "router_4.h"
#include "router_3.h"
#include "cache.h"
#include "ram.h"
#include "dummy.h"
#include "estimador.h"
//#include "cpu.h"

SC_MODULE(Top)   
{   
    Controler *cpu;   
    Cache     *cache;
    Estimador *IP;
    Ram       *ram;
    Router    *router_1;
    Router2    *router_2;
    Router3    *router_3;
    Router4    *router_4;

    SC_CTOR(Top)
    {
        IP        = new Estimador ("IP");   
        cpu       = new Controler ("cpu");
        
        router_1  = new Router    ("router_1");
        router_2  = new Router2   ("router_2");
        router_3  = new Router3   ("router_3");
        router_4  = new Router4   ("router_4");

        ram       = new Ram       ("ram");   
        cache     = new Cache     ("cache");   

        cpu      ->socket_initiator.bind(  router_4->socket_target_C); //CPU
        router_4 ->socket_initiator_C.bind(cpu     ->socket_target);

        router_1 ->socket_initiator_C.bind(router_4->socket_target_A);
        router_4 ->socket_initiator_A.bind(router_1->socket_target_C);

        cache    ->socket_initiator.bind(  router_1->socket_target_A); //CACHE
        router_1 ->socket_initiator_A.bind(cache   ->socket_target);

        router_2 ->socket_initiator_A.bind(router_1->socket_target_B);
        router_1 ->socket_initiator_B.bind(router_2->socket_target_A);

        ram      ->socket_initiator.bind(  router_2->socket_target_B); //RAM
        router_2 ->socket_initiator_B.bind(ram     ->socket_target);

        router_3 ->socket_initiator_B.bind(router_2->socket_target_E);
        router_2 ->socket_initiator_E.bind(router_3->socket_target_B);

        IP       ->socket_initiator.bind(  router_3->socket_target_E); //IP
        router_3 ->socket_initiator_E.bind(IP      ->socket_target);

        router_3 ->socket_initiator_C.bind(router_4->socket_target_E);
        router_4 ->socket_initiator_E.bind(router_3->socket_target_C);
    }
};

#endif
