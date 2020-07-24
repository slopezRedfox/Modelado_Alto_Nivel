#ifndef TOP_H
#define TOP_H

#include "initiator.h"
#include "target.h"
#include "router.h"

SC_MODULE(Top)   
{   
  Initiator *initiator;   
  Memory    *cache;
  Memory    *memory;      
  Router    *router;

  SC_CTOR(Top){

    /*
    // Instantiate components   
    initiator = new Initiator("initiator");   
    memory    = new Memory   ("memory");   
   
    // Bind initiator socket to target socket   
    initiator->socket.bind(memory->socket);
    
    */
    initiator = new Initiator("initiator");
    router    = new Router   ("router");
    memory    = new Memory   ("memory");   
    cache     = new Memory   ("cache");   

    initiator->socket.bind(router->socket_to_initiator);
    router->socket_to_target_1.bind(cache->socket);
    router->socket_to_target_2.bind(memory->socket);
  }
};

#endif
