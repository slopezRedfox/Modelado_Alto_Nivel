#ifndef TOP_H
#define TOP_H

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "initiator.h"
#include "router.h"
#include "cache.h"
#include "ram.h"
#include "dummy.h"

SC_MODULE(Top)   
{   
    Controler *controler;   
    Cache     *cache;
    Dummy     *dummy_1;
    Dummy     *dummy_2;
    Ram       *ram;
    Router    *router_1;
    Router    *router_2;
    //Router    *router_3;
    Router    *router_4;

    SC_CTOR(Top)
    {
        controler = new Controler("controler");
        router_1  = new Router   ("router_1");
        router_2  = new Router   ("router_2");
        //router_3  = new Router   ("router_3");
        router_4  = new Router   ("router_4");
        dummy_1   = new Dummy    ("dummy_1");   
        dummy_2   = new Dummy    ("dummy_2");   
        ram       = new Ram      ("ram");   
        cache     = new Cache    ("cache");   

        controler->socket_initiator.bind(  router_4->socket_target_C);
        router_4 ->socket_initiator_C.bind(controler->socket_target);

        dummy_1  ->socket_initiator.bind(  router_4->socket_target_B);
        router_4 ->socket_initiator_B.bind(dummy_1->socket_target);

        router_1 ->socket_initiator_C.bind(router_4->socket_target_A);
        router_4 ->socket_initiator_A.bind(router_1->socket_target_C);

        cache    ->socket_initiator.bind(  router_1->socket_target_A);
        router_1 ->socket_initiator_A.bind(cache->socket_target);

        router_2 ->socket_initiator_A.bind(router_1->socket_target_B);
        router_1 ->socket_initiator_B.bind(router_2->socket_target_A);

        ram      ->socket_initiator.bind(  router_2->socket_target_B);
        router_2 ->socket_initiator_B.bind(ram->socket_target);

        dummy_2  ->socket_initiator.bind(  router_2->socket_target_C);
        router_2 ->socket_initiator_C.bind(dummy_2->socket_target);
    }
};

#endif
